#include "crypto.h"

#include <string.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/aes.h>
#include <mbedtls/cipher.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>
#include <mbedtls/base64.h>
#include "argon2/include/argon2.h"

#include "hash.h"
#include "byteorder.h"
#include "rdrand.h"

#define GOTO(LABEL)                                                       \
    do                                                                    \
    {                                                                     \
        printf("GOTO: %s(%u): %s()\n", __FILE__, __LINE__, __FUNCTION__); \
        fflush(stdout);                                                   \
        goto LABEL;                                                       \
    }                                                                     \
    while (0)

void vic_luks_random(void* data, size_t size)
{
    size_t r = size;
    uint8_t* p = (uint8_t*)data;

    /* ATTN: Consider using rdseed when supported? */

    while (r >= sizeof(uint64_t))
    {
        uint64_t x = vic_rdrand();
        memcpy(p, &x, sizeof(uint64_t));
        r -= sizeof(uint64_t);
        p += sizeof(uint64_t);
    }

    if (r)
    {
        uint64_t x = vic_rdrand();
        memcpy(p, &x, r);
    }
}

int vic_luks_pbkdf2(
    const void* password,
    size_t password_size,
    const uint8_t* salt,
    size_t salt_size,
    uint32_t iterations,
    const char* hash_spec,
    void* key,
    size_t key_size)
{
    int ret = -1;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t* info;
    mbedtls_md_type_t md_type;

    mbedtls_md_init(&ctx);

    if (!password || !password_size || !salt || !salt_size || !key || !key_size)
        GOTO(done);

    if (!hash_spec)
        GOTO(done);

    if (key_size > UINT_MAX)
        GOTO(done);

    if (strcmp(hash_spec, VIC_HASH_SPEC_SHA1) == 0)
        md_type = MBEDTLS_MD_SHA1;
    else if (strcmp(hash_spec, VIC_HASH_SPEC_SHA256) == 0)
        md_type = MBEDTLS_MD_SHA256;
    else if (strcmp(hash_spec, VIC_HASH_SPEC_SHA512) == 0)
        md_type = MBEDTLS_MD_SHA512;
    else if (strcmp(hash_spec, VIC_HASH_SPEC_RIPEMD160) == 0)
        md_type = MBEDTLS_MD_RIPEMD160;
    else
        GOTO(done);

    if (!(info = mbedtls_md_info_from_type(md_type)))
        GOTO(done);

    if (mbedtls_md_setup(&ctx, info, 1) != 0)
        GOTO(done);

    if (mbedtls_pkcs5_pbkdf2_hmac(
        &ctx,
        password,
        password_size,
        salt,
        salt_size,
        iterations,
        key_size,
        key) != 0)
    {
        GOTO(done);
    }

    ret = 0;

done:
    mbedtls_md_free(&ctx);

    return ret;
}

static void _compute_xor(
    void* x_,
    const void* lhs_,
    const void* rhs_,
    size_t size)
{
    uint8_t* x = (uint8_t*)x_;
    const uint8_t* lhs = (const uint8_t*)lhs_;
    const uint8_t* rhs = (const uint8_t*)rhs_;

    /* x = lhs ^ rhs */
    while (size--)
        *x++ = *lhs++ ^ *rhs++;
}

static int _diffuse(const char* hash_spec, uint8_t* data, uint32_t bytes)
{
    int ret = -1;
    uint32_t i;
    uint32_t hash_size;
    vic_hash_type_t hash_type;
    uint32_t pad;
    vic_hash_ctx_t ctx;
    bool free_ctx = false;

    if ((hash_size = vic_hash_size(hash_spec)) == (uint32_t)-1)
        GOTO(done);

    if ((hash_type = vic_hash_type(hash_spec)) == VIC_HASH_NONE)
        GOTO(done);

    if (vic_hash_init(&ctx, hash_type) != 0)
        GOTO(done);
    free_ctx = true;

    pad = (bytes % hash_size) != 0;

    for (i = 0; i < bytes / hash_size + pad; i++)
    {
        vic_hash_t hash;
        uint32_t iv;
        uint32_t size;

        if (vic_hash_start(&ctx) != 0)
            GOTO(done);

        iv = vic_swap_u32(i);

        if (vic_hash_update(&ctx, &iv, sizeof(iv)) != 0)
            GOTO(done);

        size = hash_size;

        if (size > bytes - i * hash_size)
            size = bytes - i * hash_size;

        if (vic_hash_update(&ctx, data + i * hash_size, size) != 0)
            GOTO(done);

        if (vic_hash_finish(&ctx, &hash) != 0)
            GOTO(done);

        memcpy(data + i * hash_size, &hash, size);
    }

    ret = 0;

done:

    if (free_ctx)
        vic_hash_free(&ctx);

    return ret;
}

int vic_luks_af_merge(
    uint64_t key_bytes,
    uint64_t stripes,
    const char* hash_spec,
    const uint8_t* split_key,
    vic_key_t* key)
{
    uint32_t i;

    memset(key, 0, key_bytes);

    /* For each stripe */
    for (i = 0; i < stripes; i++)
    {
        const uint8_t* stripe;

        /* Get pointer to current stripe */
        stripe = split_key + (i * key_bytes);

        /* XOR current stripe into the master key */
        _compute_xor(key, key, stripe, key_bytes);

        /* Diffuse all but the last stripe */
        if (i + 1 != stripes)
        {
            if (_diffuse(hash_spec, key->buf, key_bytes) != 0)
                return -1;
        }
    }

    return 0;
}

int vic_luks_argon2i(
    const void* password,
    size_t password_size,
    const uint8_t* salt,
    size_t salt_size,
    uint32_t time,
    uint32_t memory,
    uint32_t cpus,
    void* key,
    size_t key_size)
{
    int ret = -1;
    int r;
    argon2_context context =
    {
        .flags = ARGON2_DEFAULT_FLAGS,
        .version = ARGON2_VERSION_NUMBER,
        .t_cost = (uint32_t)time,
        .m_cost = (uint32_t)memory,
        .lanes = (uint32_t)cpus,
        .threads = (uint32_t)cpus,
        .out = (uint8_t*)key,
        .outlen = (uint32_t)key_size,
        .pwd = (uint8_t*)password,
        .pwdlen = (uint32_t)password_size,
        .salt = (uint8_t*)salt,
        .saltlen = (uint32_t)salt_size,
    };

    switch ((r = argon2_ctx(&context, Argon2_i)))
    {
        case ARGON2_OK:
            break;
        default:
        {
            printf("r=%d\n", r);
            goto done;
        }
    }

    ret = 0;

done:
    return ret;
}

int vic_luks_argon2id(
    const void* password,
    size_t password_size,
    const uint8_t* salt,
    size_t salt_size,
    uint32_t time,
    uint32_t memory,
    uint32_t cpus,
    void* key,
    size_t key_size)
{
    int ret = -1;
    int r;
    argon2_context context =
    {
        .flags = ARGON2_DEFAULT_FLAGS,
        .version = ARGON2_VERSION_NUMBER,
        .t_cost = (uint32_t)time,
        .m_cost = (uint32_t)memory,
        .lanes = (uint32_t)cpus,
        .threads = (uint32_t)cpus,
        .out = (uint8_t*)key,
        .outlen = (uint32_t)key_size,
        .pwd = (uint8_t*)password,
        .pwdlen = (uint32_t)password_size,
        .salt = (uint8_t*)salt,
        .saltlen = (uint32_t)salt_size,
    };

    switch ((r = argon2_ctx(&context, Argon2_id)))
    {
        case ARGON2_OK:
            break;
        default:
        {
            printf("r=%d\n", r);
            goto done;
        }
    }

    ret = 0;

done:
    return ret;
}

static int _gen_split_material(
    const char* hash_spec,
    uint8_t* src,
    uint8_t* prev,
    uint32_t keylen,
    uint32_t stripes)
{
    int ret = -1;

    for (uint32_t i = 0; i < stripes - 1; i++)
    {
        /* dX = DIFFUSE(prev XOR stripeX) */
        {
            _compute_xor(prev, prev, src + i * keylen, keylen);

            if (_diffuse(hash_spec, prev, keylen) != 0)
                GOTO(done);
        }
    }

    ret = 0;

done:
    return ret;
}

int vic_luks_af_split(
    const char* hash_spec,
    const vic_key_t* key,
    size_t key_size,
    uint32_t stripes,
    uint8_t* split_key)
{
    int ret = -1;
    vic_key_t prev;

    if (key_size > sizeof(prev))
        GOTO(done);

    memset(&prev, 0, sizeof(prev));

    vic_luks_random(split_key, key_size * stripes);

    if (_gen_split_material(
        hash_spec, split_key, prev.buf, key_size, stripes) != 0)
    {
        GOTO(done);
    }

    _compute_xor(&prev, &prev, key, key_size);
    memcpy(split_key + (stripes - 1) * key_size, &prev, key_size);

    ret = 0;

done:

    return ret;
}

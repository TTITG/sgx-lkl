#include "vic.h"

const char* vic_result_string(vic_result_t result)
{
    switch (result)
    {
        case VIC_OK:
            return "VIC_OK";
        case VIC_FAILED:
            return "VIC_FAILED";
        case VIC_BAD_VERSION:
            return "VIC_BAD_VERSION";
        case VIC_UNEXPECTED:
            return "VIC_UNEXPECTED";
        case VIC_BAD_PARAMETER:
            return "VIC_BAD_PARAMETER";
        case VIC_BAD_DEVICE:
            return "VIC_BAD_DEVICE";
        case VIC_OUT_OF_MEMORY:
            return "VIC_OUT_OF_MEMORY";
        case VIC_NOT_FOUND:
            return "VIC_NOT_FOUND";
        case VIC_OUT_OF_BOUNDS:
            return "VIC_OUT_OF_BOUNDS";
        case VIC_KEY_TOO_BIG:
            return "VIC_KEY_TOO_BIG";
        case VIC_DEVICE_COUNT_FAILED:
            return "VIC_DEVICE_COUNT_FAILED";
        case VIC_DEVICE_GET_FAILED:
            return "VIC_DEVICE_GET_FAILED";
        case VIC_DEVICE_PUT_FAILED:
            return "VIC_DEVICE_PUT_FAILED";
        case VIC_DEVICE_TOO_SMALL:
            return "VIC_DEVICE_TOO_SMALL";
        case VIC_HEADER_READ_FAILED:
            return "VIC_HEADER_READ_FAILED";
        case VIC_KEY_MATERIAL_READ_FAILED:
            return "VIC_KEY_MATERIAL_READ_FAILED";
        case VIC_HEADER_WRITE_FAILED:
            return "VIC_HEADER_WRITE_FAILED";
        case VIC_KEY_MATERIAL_WRITE_FAILED:
            return "VIC_KEY_MATERIAL_WRITE_FAILED";
        case VIC_KEY_LOOKUP_FAILED:
            return "VIC_KEY_LOOKUP_FAILED";
        case VIC_PBKDF2_FAILED:
            return "VIC_PBKDF2_FAILED";
        case VIC_ENCRYPT_FAILED:
            return "VIC_ENCRYPT_FAILED";
        case VIC_DECRYPT_FAILED:
            return "VIC_DECRYPT_FAILED";
        case VIC_AFMERGE_FAILED:
            return "VIC_AFMERGE_FAILED";
        case VIC_AFSPLIT_FAILED:
            return "VIC_AFSPLIT_FAILED";
        case VIC_EOF:
            return "VIC_EOF";
        case VIC_UNSUPPORTED:
            return "VIC_UNSUPPORTED";
        case VIC_BUFFER_TOO_SMALL:
            return "VIC_BUFFER_TOO_SMALL";
        case VIC_UNKNOWN_KEYSLOT_TYPE:
            return "VIC_UNKNOWN_KEYSLOT_TYPE";
        case VIC_UNKNOWN_KDF_TYPE:
            return "VIC_UNKNOWN_KDF_TYPE";
        case VIC_DIGEST_NOT_FOUND:
            return "VIC_DIGEST_NOT_FOUND";
        case VIC_ARGON2I_FAILED:
            return "VIC_ARGON2I_FAILED";
        case VIC_ARGON2ID_FAILED:
            return "VIC_ARGON2ID_FAILED";
        case VIC_UNSUPPORTED_DIGEST_TYPE:
            return "VIC_UNSUPPORTED_DIGEST_TYPE";
        case VIC_NUM_CPUS_FAILED:
            return "VIC_NUM_CPUS_FAILED";
        case VIC_OUT_OF_KEYSLOTS:
            return "VIC_OUT_OF_KEYSLOTS";
        case VIC_BAD_UUID:
            return "VIC_BAD_UUID";
        case VIC_LAST_KEYSLOT:
            return "VIC_LAST_KEYSLOT";
        case VIC_UNSUPPORTED_INTEGRITY_JOURNALING:
            return "VIC_UNSUPPORTED_INTEGRITY_JOURNALING";
        case VIC_DEVICE_OPEN_FAILED:
            return "VIC_DEVICE_OPEN_FAILED";
        case VIC_PATH_TOO_LONG:
            return "VIC_PATH_TOO_LONG";
        case VIC_FAILED_TO_GET_LOOP_DEVICE:
            return "VIC_FAILED_TO_GET_LOOP_DEVICE";
        case VIC_UNSUPPORTED_CIPHER:
            return "VIC_UNSUPPORTED_CIPHER";
        case VIC_READ_FAILED:
            return "VIC_READ_FAILED";
        case VIC_WRITE_FAILED:
            return "VIC_WRITE_FAILED";
        case VIC_STAT_FAILED:
            return "VIC_STAT_FAILED";
        case VIC_NOT_BLOCK_MULTIPLE:
            return "VIC_NOT_BLOCK_MULTIPLE";
        case VIC_FILE_TOO_SMALL:
            return "VIC_FILE_TOO_SMALL";
        case VIC_OPEN_FAILED:
            return "VIC_OPEN_FAILED";
        case VIC_SEEK_FAILED:
            return "VIC_SEEK_FAILED";
        case VIC_IOCTL_FAILED:
            return "VIC_IOCTL_FAILED";
        case VIC_BAD_SIGNATURE:
            return "VIC_BAD_SIGNATURE";
    };

    /* Unreachable */
    return "UNKNOWN";
}
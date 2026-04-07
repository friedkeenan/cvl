#pragma once

#if defined(__GNUC__) and not defined(__clang__)

#define CVL_PUSH_DISABLE_FRIEND_WARNING \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wnon-template-friend\"")

#define CVL_POP_DISABLE_FRIEND_WARNING \
    _Pragma("GCC diagnostic pop")

#else

#define CVL_PUSH_DISABLE_FRIEND_WARNING
#define CVL_POP_DISABLE_FRIEND_WARNING

#endif

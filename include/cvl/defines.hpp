#pragma once

#if defined(__GNUC__) and not defined(__clang__)

#define CVL_DISABLE_FRIEND_WARNING(tokens) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wnon-template-friend\"") \
    tokens \
    _Pragma("GCC diagnostic pop")

#else

#define CVL_DISABLE_FRIEND_WARNING(tokens) tokens

#endif

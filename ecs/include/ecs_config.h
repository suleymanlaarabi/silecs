#ifndef ECS_CONFIG_H
    #define ECS_CONFIG_H
    #include <stddef.h>

    #ifndef ECS_LIKELY
    #  if defined(__GNUC__) || defined(__clang__)
    #    define ECS_LIKELY(x)   __builtin_expect(!!(x), 1)
    #    define ECS_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #  else
    #    define ECS_LIKELY(x)   (x)
    #    define ECS_UNLIKELY(x) (x)
    #  endif
    #endif

    #define ECS_INLINE static inline __attribute__((always_inline))

#endif

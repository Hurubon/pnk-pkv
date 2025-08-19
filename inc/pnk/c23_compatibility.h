#ifndef PNK_C23_COMPATIBILITY
#define PNK_C23_COMPATIBILITY

    #if __STDC_VERSION__ >= 202311L
        #define PNK_NODISCARD                [[nodiscard]]
        #define PNK_NODISCARD_REASON(reason) [[nodiscard(reason)]]
    #elif defined(__GNUC__) || defined(__clang__)
        #define PNK_NODISCARD           __attribute__((warn_unused_result))
        #define PNK_NODISCARD_REASON(_) __attribute__((warn_unused_result))
    #elif defined(_MSC_VER)
        #include <sal.h>
        #define PNK_NODISCARD           _Check_return
        #define PNK_NODISCARD_REASON(_) _Check_return
    #else
        #define PNK_NODISCARD
        #define PNK_NODISCARD_REASON(_)
    #endif

    #if __STDC_VERSION__ >= 202311L
        #define PNK_MAYBE_UNUSED [[maybe_unused]]
    #elif defined(__GNUC__) || defined(__clang__)
        #define PNK_MAYBE_UNUSED __attribute__((unused))
    #else
        #define PNK_MAYBE_UNUSED
    #endif

#endif /* PNK_C23_COMPATIBILITY */

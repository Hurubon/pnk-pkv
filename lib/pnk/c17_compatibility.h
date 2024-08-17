#ifndef PNK_C17_COMPATIBILITY_HEADER
#define PNK_C17_COMPATIBILITY_HEADER

/************************************** Attributes **************************************/
#if defined(__STDC_VERSION__) && __STDC_VERSION__ == 202311L
    #define PNK_NODISCARD [[nodiscard]]
#elif defined __GNUC__
    #define PNK_NODISCARD __attribute__((warn_unused_result))
#else
    #define PNK_NODISCARD
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ == 202311L
    #define PNK_MAYBE_UNUSED [[maybe_unused]]
#elif defined __GNUC__
    #define PNK_MAYBE_UNUSED __attribute__((unused))
#else
    #define PNK_MAYBE_UNUSED
#endif

#endif /* PNK_C17_COMPATIBILITY_HEADER */

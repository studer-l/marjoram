#pragma once
#if not defined(MARJORAM_ALLOW_DISCARD) && defined(__has_cpp_attribute) && \
    __has_cpp_attribute(nodiscard)
#define MARJORAM_NODISCARD [[nodiscard]]
#else
#define MARJORAM_NODISCARD
#endif

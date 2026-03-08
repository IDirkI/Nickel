#pragma once

#include "types.hpp"
#include "log.hpp"

#ifdef NDEBUG
    #define NI_ASSERT(expr, msg)
#else
    #define NI_ASSERT(expr, msg)    \
        do {    \
            if(!(expr)) { \
                nickel::assert::assertionFail(#expr, __FILE__, __LINE__, __func__, msg);    \
            }   \
        } while(0)
        
#endif

namespace nickel::assert {
    // ==========================
    //          Assert
    // ==========================

    // ───── Function Prototypes ────────────────────────────────────────────────────────────
    [[noreturn]] void assertionFail(const char *strExpr, const char *file, size_t line, const char *func, const char *msg);
}   // namespace nickel::assert
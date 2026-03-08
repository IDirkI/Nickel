#include "assert.hpp"

#include "log.hpp"

extern "C" {
    #include "stm32f4xx_hal.h"
}

namespace nickel::assert {

// ───── Function Implementations ────────────────────────────────────────────────────────────
[[noreturn]] void assertionFail(const char *strExpr, const char *file, size_t line, const char *func, const char *msg) {
    __asm volatile ("cpsid i" ::: "memory");

    LOG_ERROR("──ASSERTION─FAILED── | \"%s\"", strExpr);
    LOG_ERROR("                     | at %s:%d in %s()", file, line, func);
    if(msg != nullptr) {
        LOG_ERROR("                     | Msg: %s", msg);
    }
    
    while(true) {}
}

}   // namespace nickel::assert
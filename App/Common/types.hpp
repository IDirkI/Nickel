#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace nickel {
// ───── Define Util ────────────────────────────────────────────────────────────
#define UNSUED(x)   ((void)(x))

// ───── Templated general util ────────────────────────────────────────────────────────────
template<typename T, size_t N>
constexpr size_t array_size(T (&)[N]) { return N; }

template<typename T>
constexpr T clamp(T x, T lo, T hi) { 
    static_assert(std::is_arithmetic_v<T>, ">> clamp() supports only numerical types");
    return ((x < lo) ?lo :((x > hi) ? hi : x));
}

// ───── Numerical Aliases ────────────────────────────────────────────────────────────
typedef float   float32_t;
typedef double  float64_t;
typedef size_t  index_t;

// ───── Enums ────────────────────────────────────────────────────────────
enum class Status : uint8_t{
    OK          = 0,
    ERR         = 1,
    BUSY        = 2,
    TIMEOUT     = 3,
    INVALID_ARG = 4,
    NOT_READY   = 5,  
};

} // namespace nickel
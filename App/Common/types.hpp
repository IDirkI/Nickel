#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

// --- Define Util ---
#define UNSUED(x)   ((void)(x))

// --- Templated Util ---
template<typename T, size_t N>
    constexpr size_t array_size(T (&)[N]) { return N; }

template<typename T>
    constexpr T clamp(T x, T lo, T hi) { 
        static_assert(std::is_arithmetic_v<T>, ">> clamp() supports only numerical types");
        return ((x < lo) ?lo :((x > hi) ? hi : x));
    }

// --- Numerical Aliases ---
typedef float   float32;
typedef double  float64;

// --- Enums ---
enum class Status : uint8_t{
    OK          = 0,
    ERR         = 1,
    BUSY        = 2,
    TIMEOUT     = 3,
    INVALID_ARG = 4,
    NOT_READY   = 5,  
};
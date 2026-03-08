#pragma once

#include <cstdarg>

#include "types.hpp"

// ───── Log Level Defines ────────────────────────────────────────────────────────────
#define LOGLEVEL_NONE   0
#define LOGLEVEL_ERR    1
#define LOGLEVEL_WARN   2
#define LOGLEVEL_INFO   3
#define LOGLEVEL_DBG    4

#ifndef LOG_LEVEL
#define LOG_LEVEL LOGLEVEL_DBG
#endif

// ───── Logging Macros ────────────────────────────────────────────────────────────
#if LOG_LEVEL >= LOGLEVEL_ERR
    #define LOG_ERROR(fmt, ...) log::print("\033[31m[ERR]  !!\033[0m " fmt "\r\n", ##__VA_ARGS__)
#else
    #define LOG_ERROR(fmt, ...)
#endif

#if LOG_LEVEL >= LOGLEVEL_WRN
    #define LOG_WARN(fmt, ...) log::print("\033[33m[WARN]  !\033[0m " fmt "\r\n", ##__VA_ARGS__)
#else
    #define LOG_WARN(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
    #define LOG_INFO(fmt, ...)  log::print("\033[36m[INFO]   \033[0m " fmt "\r\n", ##__VA_ARGS__)
#else
    #define LOG_INFO(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DBG
    #define LOG_DEBUG(fmt, ...) log::print("\033[35m[DBG]   >>\033[0m " fmt "\r\n", ##__VA_ARGS__)
#else
    #define LOG_DEBUG(fmt, ...)
#endif

namespace nickel::log {

// ==========================
//           Log
// ==========================

// ───── Enums ────────────────────────────────────────────────────────────
/**
 *  @brief Level of logging to execute
 */
enum class LogLevel {
    NONE    = 0,
    ERR     = 1,
    WARN    = 2,
    INFO    = 3,
    DBG     = 4,
}; 

// ───── Function Prototypes ────────────────────────────────────────────────────────────
Status init(void);
Status print(const char *fmt, ...);
Status hex_dump(const uint8_t *data, uint16_t length, const char *label);
Status array_u16(const uint16_t *data, uint16_t length, const char *label);
Status array_f32(const float32_t *data, uint16_t length, const char *label);

} // namespace nickel::log
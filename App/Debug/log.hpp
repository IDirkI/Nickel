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
// Error
#if LOG_LEVEL >= LOGLEVEL_ERR 
    #define LOG_ERROR(fmt, ...) log::print("\033[31m[ERR]  !!\033[0m " fmt "\r\n", ##__VA_ARGS__)
#else
    #define LOG_ERROR(fmt, ...)
#endif

// Warning
#if LOG_LEVEL >= LOGLEVEL_WARN
    #define LOG_WARN(fmt, ...)  log::print("\033[33m[WARN]  !\033[0m " fmt "\r\n", ##__VA_ARGS__)
#else
    #define LOG_WARN(fmt, ...)
#endif

// Info
#if LOG_LEVEL >= LOGLEVEL_INFO
    #define LOG_INFO(fmt, ...)      log::print("\033[36m[INFO]\033[0m    " fmt "\r\n", ##__VA_ARGS__)
    #define LOG_STATUS(label, s)    log::print("\033[93m[STAT]    =>\033[0m %s ── %s\r\n", label, log::status_str(s))
    #define LOG_SEPARATOR(label)    log::print("\033[90m───── %s ────────────────────────────────────────────────────────\033[0m\r\n", label)
    #define LOG_BOOT(label)         log::print("\033[97m[BOOT] **\033[0m %s\r\n", label)
    #define LOG_READING(interval_ms, label, fmt, ...)                      \
        do {                                                                \
            static uint32_t _last = 0;                                      \
            uint32_t _now = HAL_GetTick();                                  \
            if((_now - _last) >= (interval_ms)) {                           \
                _last = _now;                                               \
                log::print("\033[92m[READ]  ~\033[0m [%lums] ─ %s: " fmt "\r\n", _now, label, ##__VA_ARGS__); \
            }                                                               \
        } while(0)
    #define LOG_READING_NL(interval_ms, fmt, ...)                           \
        do {                                                                \
            static uint32_t _last = 0;                                      \
            uint32_t _now = HAL_GetTick();                                  \
            if((_now - _last) >= (interval_ms)) {                           \
                _last = _now;                                               \
                log::print("\033[92m[READ]  ~\033[0m [%lums] ─ " fmt "\r\n", _now, ##__VA_ARGS__); \
            }                                                               \
        } while(0)
#else
    #define LOG_INFO(fmt, ...)
    #define LOG_STATUS(label, s)
    #define LOG_SEPARATOR(label)
    #define LOG_BOOT(label)
    #define LOG_READING(interval_ms, label, fmt, ...)
    #define LOG_READING_NL(interval_ms, fmt, ...)
#endif

// Debug
#if LOG_LEVEL >= LOGLEVEL_DBG
    #define LOG_DEBUG(fmt, ...) log::print("\033[35m[DBG]   >>\033[0m " fmt "\r\n", ##__VA_ARGS__)
    #define LOG_TRACE()         log::print("\033[94m[TRC]  -->\033[0m %s() @ line %d\r\n", __func__, __LINE__)
#else
    #define LOG_DEBUG(fmt, ...)
    #define LOG_TRACE()
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
Status log_init(void);
Status print(const char *fmt, ...);
Status hex_dump(const uint8_t *data, uint16_t length, const char *label);
Status array_u16(const uint16_t *data, uint16_t length, const char *label);
Status array_f32(const float32_t *data, uint16_t length, const char *label);

const char *status_str(Status s);

} // namespace nickel::log
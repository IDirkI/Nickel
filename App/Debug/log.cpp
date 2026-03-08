#include "log.hpp"
#include "types.hpp"

#include <cstdio>
#include <cstring>

extern "C" {
    #include "stm32f4xx_hal.h"
    extern UART_HandleTypeDef huart3;
}

// --- Private Helpers ---
namespace {
    constexpr uint16_t BUFFER_SIZE = 256;
    static char tx_buff[2][BUFFER_SIZE];
    static uint8_t  tx_buffIndex = 0;
    static bool flag_initialized = false;

    // Write a char to SWO
    void swo_write(const char *str, uint16_t length) {
        for (uint16_t i = 0; i < length; i++) {
            ITM_SendChar(static_cast<uint32_t>(str[i]));
        }
    }

    // Transmit non-blocking message through USART3
    Status uart_write(const char *str, uint16_t length) {
        if(HAL_UART_GetState(&huart3) != HAL_UART_STATE_READY) { return Status::BUSY; }

        uint8_t index = tx_buffIndex;
        tx_buffIndex  = (tx_buffIndex + 1) % 2;

        memcpy(tx_buff[tx_buffIndex], str, length);

        HAL_StatusTypeDef result = HAL_UART_Transmit_DMA(&huart3, reinterpret_cast<const uint8_t *>(tx_buff[tx_buffIndex]), length);

        switch(result) {
            case (HAL_OK):      { return Status::OK; }
            case (HAL_BUSY):    { return Status::BUSY; }
            case (HAL_TIMEOUT): { return Status::TIMEOUT; }
            default:            { return Status::ERR; }
        }
    }
};

// --- Function Definitions ---
namespace Log {

/**
 *  @brief Initialize the logging system (must be called before any other Log functions)
 *  @return Status of initialization
 */
Status init(void) {
    flag_initialized = true;
    return Status::OK;
}

Status print(const char *fmt, ...) {
    if(!flag_initialized) { return Status::NOT_READY; }

    char tmp_buff[BUFFER_SIZE];

    va_list args;
    va_start(args, fmt);
    uint32_t length = vsnprintf(tmp_buff, BUFFER_SIZE, fmt, args);
    va_end(args);

    if(length <= 0) { return Status::ERR; }
    if(length >= BUFFER_SIZE) { length = BUFFER_SIZE - 1; }  // truncated but safe

    // Always write to SWO — never blocked
    swo_write(tmp_buff, static_cast<uint16_t>(length));

    // Write to USART3 — drops silently if busy
    return uart_write(tmp_buff, static_cast<uint16_t>(length));
}

Status hex_dump(const uint8_t *data, uint16_t length, const char *label) {
    if(!flag_initialized) { return Status::NOT_READY; }
    if(data == nullptr) { return Status::INVALID_ARG; }

    Log::print("[HEX] %s (%u bytes):\r\n", label, length);

    for(uint16_t i = 0; i < length; i++) {
        Log::print("%02X ", data[i]);
        if ((i + 1) % 16 == 0) { Log::print("\r\n"); }
    }
    if(length % 16 != 0) Log::print("\r\n");

    return Status::OK;
}

Status array_u16(const uint16_t *data, uint16_t length, const char *label) {
    if(!flag_initialized) { return Status::NOT_READY; }
    if(data == nullptr) { return Status::INVALID_ARG; }

    Log::print("[U16] %s: [ ", label);
    for (uint16_t i = 0; i < length; i++) {
        Log::print("%u%s", data[i], (i < length - 1) ?", " :" ]\r\n");
    }
    return Status::OK;
}

Status array_f32(const float32 *data, uint16_t length, const char *label) {
    if(!flag_initialized) { return Status::NOT_READY; }
    if(data == nullptr) { return Status::INVALID_ARG; }

    Log::print("[F32] %s: [ ", label);
    for (uint16_t i = 0; i < length; i++) {
        Log::print("%.4f%s", data[i], i < length - 1 ? ", " : " ]\r\n");
    }
    return Status::OK;
}

}; // Log
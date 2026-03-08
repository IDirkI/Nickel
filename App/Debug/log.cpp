#include "log.hpp"

#include <cstdio>
#include <cstring>

#include "types.hpp"
#include "ring_buffer.hpp"

extern "C" {
    #include "stm32f4xx_hal.h"
    extern UART_HandleTypeDef huart3;
}

using namespace nickel;
using namespace nickel::common;
using namespace nickel::log;

namespace nickel::log {

// ───── Private Helpers ────────────────────────────────────────────────────────────
constexpr uint16_t BUFFER_SIZE = 256;
static RingBuff<512> txBuff;

static size_t txIncoming = 0;
static bool flag_initialized = false;
static bool flag_dmaBusy = false;

// Write a char to SWO
void swo_write(const char *str, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        ITM_SendChar(static_cast<uint32_t>(str[i]));
    }
}

// Drain the ring buffer into DMA 
void uart_drain(void) {
    if(flag_dmaBusy) { return; }
    if(txBuff.isEmpty()) { return; }

    index_t length = 0;
    const uint8_t *ptr = txBuff.peek_data(length);

    if(ptr == nullptr) { return; }
    if(length == 0) { return; }  

    txIncoming = length;
    flag_dmaBusy = true;
    HAL_UART_Transmit_DMA(&huart3,  const_cast<uint8_t *>(ptr), static_cast<uint16_t>(length));
}

// Transmit non-blocking message through USART3
Status uart_write(const char *str, uint16_t length) {
    Status status = txBuff.push(reinterpret_cast<const uint8_t *>(str), length);
    if(status != Status::OK) { return status; }
    uart_drain();

    return Status::OK;
}

// ───── Function Implementations ────────────────────────────────────────────────────────────
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
    int length = vsnprintf(tmp_buff, BUFFER_SIZE, fmt, args);
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

    log::print("[HEX] %s (%u bytes):\r\n", label, length);

    for(uint16_t i = 0; i < length; i++) {
        log::print("%02X ", data[i]);
        if ((i + 1) % 16 == 0) { log::print("\r\n"); }
    }
    if(length % 16 != 0) log::print("\r\n");

    return Status::OK;
}

Status array_u16(const uint16_t *data, uint16_t length, const char *label) {
    if(!flag_initialized) { return Status::NOT_READY; }
    if(data == nullptr) { return Status::INVALID_ARG; }

    log::print("[U16] %s: [ ", label);
    for (uint16_t i = 0; i < length; i++) {
        log::print("%u%s", data[i], (i < length - 1) ?", " :" ]\r\n");
    }
    return Status::OK;
}

Status array_f32(const float32_t *data, uint16_t length, const char *label) {
    if(!flag_initialized) { return Status::NOT_READY; }
    if(data == nullptr) { return Status::INVALID_ARG; }

    log::print("[F32] %s: [ ", label);
    for (uint16_t i = 0; i < length; i++) {
        log::print("%.4f%s", data[i], i < length - 1 ? ", " : " ]\r\n");
    }
    return Status::OK;
}

} // namespace namespace::log


// ───── Weak-symbol Callbacks ────────────────────────────────────────────────────────────
extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart -> Instance != USART3) { return; }

    txBuff.consume(txIncoming);
    txIncoming = 0;
    

    flag_dmaBusy = false;
    uart_drain();
}
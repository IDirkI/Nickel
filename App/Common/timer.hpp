#pragma once

#include "types.hpp"

extern "C" {
    #include "stm32f4xx_hal.h"
    extern TIM_HandleTypeDef htim2;
}

namespace nickel::common {

// ==========================
//        Free Timer
// ==========================

// ───── Constants ────────────────────────────────────────────────────────────
constexpr uint32_t TIM2_FREQ_HZ = 50000000ULL;                  // 50 MHz
constexpr uint32_t TICKS_PER_MS = TIM2_FREQ_HZ / 1000;          // 50000 tick/ms
constexpr uint32_t TICKS_PER_US = TIM2_FREQ_HZ / 1000000;       // 50 tick/us
constexpr uint32_t NS_PER_TICK  = 1000000000UL / TIM2_FREQ_HZ;  // 20ns / tick

constexpr uint8_t TIMER_DEFAULT_CHECKPOINT_COUNT = 4;

// ───── Free function utility ────────────────────────────────────────────────────────────
/**
 *  @brief Initialize the timer. Must be called before using any other timer function
 *  @return Status of initialization
 */
inline Status timer_init(void) {
    if(HAL_TIM_Base_Start(&htim2) != HAL_OK) { return Status::ERR; }
    return Status::OK;
}

/**
 *  @brief Get the current tick count of TIM2
 *  @return Current tick count
 */
inline uint32_t ticks(void) {
    return __HAL_TIM_GET_COUNTER(&htim2);
}

/**
 *  @brief Get the current time in milliseconds since timer initialization
 *  @return Current time in milliseconds
 */
inline uint32_t millis(void) {
    return HAL_GetTick();
}

/**
 *  @brief Get the current time in microseconds since timer initialization
 *  @return Current time in microseconds
 */
inline uint32_t micro(void) {
    return ticks() / TICKS_PER_US;
}

/**
 *  @brief Get the current time in nanoseconds since timer initialization
 *  @return Current time in nanoseconds
 */
inline uint64_t nanos(void) {
    return static_cast<uint64_t>(ticks()) * NS_PER_TICK;
}

// ==========================
//            Timer
// ==========================
/**!
 *  @brief Safe, non-blocking timer
 *  @tparam C   Number of checkpoint slots availabe to keep track off (default 4)
 *  
 *  Stores the states of start_ and period_ in raw ticks with the hardware timer.
 *  Comparisons are kept uint32_t subtractions with no unit conflict & automatic roll-over.
 * 
 * 
 *  Resolution::MS ── handled by HAL_GetTick(). ~49 day roll-over time.
 * 
 *  Resolution::HW ── handled by TIM2 hardware timer ticks (20ns). ~85s roll-over time.
 */
template<uint8_t C = TIMER_DEFAULT_CHECKPOINT_COUNT>
class Timer {
    public:
        /**
         *  @brief Timer resolution mode. MS uses HAL_GetTick() and HW uses TIM2 hardware timer ticks. HW is more precise but has shorter roll-over time.
         */
        enum class Resolution {
            MS,
            HW,
        };

    private:
        uint32_t start_;
        uint32_t period_;
        uint32_t checkpoints_[C];

        Resolution res_;
        bool isRunning_;
        
        // ───── Private Helpers ────────────────────────────────────────────────────────────
        uint32_t now() const {
            return (res_ == Resolution::HW) ?ticks() :millis();
        }
    public:
        // ───── Constructors ────────────────────────────────────────────────────────────
        Timer() : start_(0), period_(0), isRunning_(false), res_(Resolution::HW), checkpoints_{} {}

        // ───── Start Time ────────────────────────────────────────────────────────────
};

}   // namespace nickel::common

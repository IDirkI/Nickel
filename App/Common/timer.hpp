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
constexpr uint64_t TIM2_FREQ_HZ = 50000000ULL;                                      // 50 MHz
constexpr uint32_t TICKS_PER_MS = static_cast<uint32_t>(TIM2_FREQ_HZ / 1000);       // 50000 tick/ms
constexpr uint32_t TICKS_PER_US = static_cast<uint32_t>(TIM2_FREQ_HZ / 1000000);    // 50 tick/us
constexpr uint32_t NS_PER_TICK  = 1000000000UL / TIM2_FREQ_HZ;                      // 20ns / tick

constexpr uint8_t TIMER_DEFAULT_TIMESTAMP_COUNT = 4;

// ───── Free function utility ────────────────────────────────────────────────────────────
/**
 *  @brief Initialize the timer. Must be called before using any other timer function
 *  @return Status of initialization
 */
inline Status timer_init(void) {
    if(HAL_TIM_Base_Start(&htim2) != HAL_OK) { return Status::ERR; }
    LOG_BOOT("TIMER");
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
inline uint32_t micros(void) {
    return ticks() / TICKS_PER_US;
}

// ───── Blocking Waits/Halts ────────────────────────────────────────────────────────────
/**
 *  @brief Blocking wait for a specified number of milliseconds
 *  @param ms Number of milliseconds to wait
 *  @note This function is blocking and will halt execution until the specified time has elapsed. Use with caution in real-time applications.
 */
inline void waitMs (uint32_t ms) {
    uint32_t start = millis();
    while((millis() - start) < ms);
}

/**
 *  @brief Blocking wait for a specified number of microseconds
 *  @param us Number of microseconds to wait
 *  @note This function is blocking and will halt execution until the specified time has elapsed. Use with caution in real-time applications.
 */
inline void waitUs (uint32_t us) {
    uint32_t start = ticks();
    while((ticks() - start) < (us * TICKS_PER_US));
}

// ==========================
//            Timer
// ==========================
/**!
 *  @brief Safe, non-blocking timer
 *  @tparam C   Number of timestamp slots availabe to keep track off (default 4)
 *  
 *  Stores the states of start_ and period_ in raw ticks with the hardware timer.
 *  Comparisons are kept uint32_t subtractions with no unit conflict & automatic roll-over.
 * 
 * 
 *  Resolution::MS ── handled by HAL_GetTick(). ~49 day roll-over time.
 * 
 *  Resolution::US ── handled by TIM2 hardware timer ticks (20ns). ~85s roll-over time.
 */
template<uint8_t C = TIMER_DEFAULT_TIMESTAMP_COUNT>
class Timer {
    public:
        /**
         *  @brief Timer resolution mode. MS uses HAL_GetTick() and US uses TIM2 hardware timer ticks.
         */
        enum class Resolution {
            MS,
            US,
        };

        /**
         *  @brief
         */
        enum class Unit {
            TICKS,
            MS,
            US
        };

    private:
        uint32_t start_;
        uint32_t period_;
        uint32_t timestamps_[C];

        bool isRunning_;
        Resolution res_;
        
        // ───── Private Helpers ────────────────────────────────────────────────────────────
        uint32_t now() const {
            return (res_ == Resolution::US) ?ticks() :millis();
        }
    public:
        // ───── Constructors ────────────────────────────────────────────────────────────
        /**
         *  @brief Construct a new Timer object. Timer is starts in a stopped.
         *  @note Timer must be started with startMs(), startUs(), or startNs() before it can be used.
         */
        Timer() : start_(0), period_(0), timestamps_{}, isRunning_(false), res_(Resolution::US)  {}

        // ───── Start Timer ────────────────────────────────────────────────────────────
        /**
         *  @brief Start the timer with a specified period in milliseconds.
         *  @param period_ms Timer period in milliseconds
         */
        void startMs(uint32_t period_ms) {
            res_ = Resolution::MS;
            period_ = period_ms;
            start_ = now();
            isRunning_ = true;
        }

        /**
         *  @brief Start the timer with a specified period in microseconds.
         *  @param period_us Timer period in microseconds
         */
        void startUs(uint32_t period_us) {
            res_ = Resolution::US;
            period_ = period_us * TICKS_PER_US;
            start_ = now();
            isRunning_ = true;
        }

        /**
         *  @brief Start the timer with a specified frequency in Hz.
         *  @param freq Timer frequency in Hz
         *  @note Maximum achievable frequency is 50 MHz (20ns period) and minimum is ~0.00006 Hz (85s period).
         */
        void startHz(float32_t freq) {
            res_ = Resolution::US;
            period_ = static_cast<uint32_t>(static_cast<float32_t>(TIM2_FREQ_HZ) / freq);
            start_ = now();
            isRunning_ = true;
        }

        // ───── Control State ────────────────────────────────────────────────────────────
        /**
         *  @brief Reset the timer to start counting from the current time. Timer must be running to reset.
         *  @note If the timer is not running, reset() will have no effect.
         */
        void reset() {
            if(!isRunning_) { return; }
            start_ = now();
            isRunning_ = true;
        }

        /**
         *  @brief Stop the timer. Timer will not count or fire while stopped. Time functions will return 0 while stopped.
         *  @note Stopping the timer does not reset the start time or period. If the timer is restarted, it will continue counting from where it left off.
         */
        void stop() {
            isRunning_ = false;
        }

        /**
         *  @brief Resume the timer. 
         *  @note If the timer is stopped, resume() will continue counting from where it left off. If the timer is already running, resume() will have no effect.
         */
        void resume() {
            isRunning_ = true;
        }

        // ───── Check State ────────────────────────────────────────────────────────────
        /**
         *  @brief Check if the timer is currently running.
         *  @return `true` if the timer is running, `false` otherwise
         */
        bool getRunning() const { return isRunning_; }

        /**
         *  @brief Get the current resolution mode of the timer.
         *  @return Current resolution mode (Resolution::MS or Resolution::US)
         */
        Resolution getResolution() const { return res_; }

        // ───── Ticking ────────────────────────────────────────────────────────────
        /**
         *  @brief One-shot, check if the timer has reached or exceeded its set period.
         *  @return `true` if the timer has fired, `false` otherwise.
         *  @note Does not reset the timer.
         */
        bool fire() const {
            if(!isRunning_) { return false; }
            return ((now() - start_) >= period_);
        }

        /**
         *  @brief Periodic, check if the timer has reached or exceeded its set period.
         *  @return `true` if the timer has fired, `false` otherwise.
         */
        bool metronome() {
            if(!isRunning_) { return false; }
            if((now() - start_) >= period_) {
                reset();
                return true;
            }
            return false;
        }

        // ───── Check Time ────────────────────────────────────────────────────────────
        /**
         *  @brief Get elapsed time since the timer was started or last reset.
         *  @param unit Unit to return the elapsed time in (default: Unit::MS)
         *  @return Elapsed time in the specified unit. Returns 0 if the timer is not running.
         */
        uint32_t time(Unit unit = Unit::MS) const {
            if(!isRunning_) { return 0; }
            uint32_t raw = now() - start_;
            switch(unit) {
                case (Unit::TICKS): { return raw; }
                case (Unit::MS):    { return raw / TICKS_PER_MS; }
                case (Unit::US):    { return raw / TICKS_PER_US; }
                default:            { return raw; }
            }
        }

        // ───── Timestamp ────────────────────────────────────────────────────────────
        /**
         *  @brief Record a timestamp in a with id. Timestamp is the current time in raw ticks.
         *  @param id ID of the timestamp slot to set [0 to C-1]
         *  @return Status of the operation. Returns `Status::OK` if the timestamp was set successfully, `Status::INVALID_ARG` if the ID is out of range, or `Status::ERR` if the timer is not running.
         */
        Status setStamp(uint8_t id) {
            if(id >= C) { return Status::INVALID_ARG; }
            timestamps_[id] = now();
            return Status::OK;
        }

        /**
         *  @brief Get the timestamp with the specified ID. Timestamp is returned in raw ticks.
         *  @param id ID of the timestamp slot to get [0 to C-1]
         *  @param out Reference to a variable where the timestamp will be stored if the operation is successful
         *  @return Status of the operation. Returns `Status::OK` if the timestamp was retrieved successfully, `Status::INVALID_ARG` if the ID is out of range, or `Status::ERR` if the timer is not running.
         */
        Status getStamp(uint8_t id, uint32_t &out) const {
            if(id >= C) { return Status::INVALID_ARG; }
            out = timestamps_[id]; 
            return Status::OK;
        }

        /**
         *  @brief Get the difference between two timestamps with the specified IDs. Difference is returned in raw ticks.
         *  @param id1 ID of the first timestamp slot [0 to C-1]
         *  @param id2 ID of the second timestamp slot [0 to C-1]
         *  @param out Reference to a variable where the timestamp difference will be stored if the operation is successful
         *  @param unit Unit to return the timestamp difference in (ticks, ms, or us)
         *  @return Status of the operation. Returns `Status::OK` if the timestamp difference was calculated successfully, `Status::INVALID_ARG` if either ID is out of range, or `Status::ERR` if the timer is not running.
         */
        Status deltaStamp(uint8_t id1, uint8_t id2, uint32_t &out,  Unit unit = Unit::MS) const {
            if(id1 >= C) { return Status::INVALID_ARG; }
            if(id2 >= C) { return Status::INVALID_ARG; }
            uint32_t raw = timestamps_[id2] - timestamps_[id1];

            switch(unit) {
                case (Unit::TICKS): { out = raw; break; }
                case (Unit::MS):    { out = raw / TICKS_PER_MS; break; }
                case (Unit::US):    { out = raw / TICKS_PER_US; break; }
                default:            { out = raw; break; }
            }

            return Status::OK;
        }

        /**
         *  @brief Get the elapsed time since the timestamp with the specified ID was set. Time is returned in raw ticks.
         *  @param id ID of the timestamp slot [0 to C-1]
         *  @param unit Unit to return the elapsed time in (ticks, ms, or us)
         *  @return Time elapsed since the timestamp with id was set, in raw ticks if the timer is running and the ID is valid, 0 otherwise.
         */
        uint32_t sinceStamp(uint8_t id, Unit unit = Unit::MS) const {
            if(id >= C) { return 0; }
            uint32_t raw = now() - timestamps_[id];

            switch(unit) {
                case (Unit::TICKS): { return raw; }
                case (Unit::MS):    { return raw / TICKS_PER_MS; }
                case (Unit::US):    { return raw / TICKS_PER_US; }
                default:            { return raw; }
            }
        }


};

}   // namespace nickel::common

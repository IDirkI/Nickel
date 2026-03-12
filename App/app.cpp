#include "app.hpp"
#include "Debug/log.hpp"
#include "Debug/assert.hpp"
#include "Common/timer.hpp"

extern "C" {
    #include "stm32f4xx_hal.h"
}

namespace nickel {
    using namespace nickel::common;
    using namespace nickel::log;

    /**
     *  @brief Initialize the application
     */
    static void init(void) {
        log_init();     // Initialize Logging
        timer_init();   // Initialize Timer
    }

    /**
     *  @brief Main application loop
     */
    static void run(void) {
        LOG_SEPARATOR("Main Loop");

        LOG_ERROR("something failed, code %d", 42);
        LOG_WARN("value out of range: %lu", 1234UL);
        LOG_INFO("booting system...");
        LOG_STATUS("timer_init", Status::OK);
        LOG_STATUS("sensor_init", Status::ERR);
        LOG_DEBUG("loop count: %d", 0);
        LOG_TRACE();
        LOG_READING(0, "IMU", "%d", 100);
        
        while(true) {
            //LOG_READING(0, "IMU", "%d", 100);
            //waitUs(1000000);
        }
    }

};  // namespace nickel


// ───── C Enrty Points ────────────────────────────────────────────────────────────
/**
 *  @brief Initialize the application
 */
void app_init(void) {
    nickel::init();
}

/**
 *  @brief Main application loop
 */
void app_run(void) {
    nickel::run();
}
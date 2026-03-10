#include "app.hpp"
#include "Debug/log.hpp"
#include "Debug/assert.hpp"
#include "Common/timer.hpp"

extern "C" {
    #include "stm32f4xx_hal.h"
}

namespace nickel {
    /**
     *  @brief Initialize the application
     */
    static void init(void) {
        log::init();
        LOG_INFO("BOOTING...");
    }

    /**
     *  @brief Main application loop
     */
    static void run(void) {
    HAL_Delay(100);

    LOG_INFO("Hello!");
    LOG_INFO("Hello!2222");

    NI_ASSERT(1 < 0, "what are we doing?");

    while(true) {

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
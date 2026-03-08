#include "app.hpp"
#include "Debug/log.hpp"

extern "C" {
    #include "stm32f4xx_hal.h"
}

/**
 *  @brief Initialize the application
 */
void app_init(void) {
    Log::init();
    LOG_INFO("BOOTING...");
}

/**
 *  @brief Main application loop
 */
void app_run(void) {
    HAL_Delay(200);
    LOG_INFO("Hello!");
    while(true) {
        
    }
}
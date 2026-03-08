# --- Make Library ---
add_library(module_debug STATIC
    log.cpp
    assert.cpp
)

target_include_directories(module_debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(module_debug 
    PRIVATE module_common
    PRIVATE stm32cubemx   
)
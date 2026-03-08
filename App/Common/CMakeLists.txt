# --- Make Library ---
add_library(module_common INTERFACE)

target_include_directories(module_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_library(free_rtos_lib src/free_rtos.c)

target_include_directories(free_rtos_lib PUBLIC include)

target_link_libraries(free_rtos_lib FreeRTOS-Kernel-Heap4 pico_stdlib)

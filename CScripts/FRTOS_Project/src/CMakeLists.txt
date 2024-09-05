add_executable(${PROJECT} main.c)

target_include_directories(${PROJECT} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link the tcp library to the main program
target_link_libraries(${PROJECT} FreeRTOS-Kernel-Heap4 tcp_client_lib mpu6050_lib)
#target_link_libraries(${PROJECT} pico_stdlib pico_cyw43_arch_none FreeRTOS-Kernel-Heap4)

pico_add_extra_outputs(${PROJECT})
pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)






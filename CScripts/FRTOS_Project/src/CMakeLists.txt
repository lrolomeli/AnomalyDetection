add_executable(${PROJECT} main.c)

# Link the adc library to the main program
target_link_libraries(${PROJECT} app_lib adctim_lib)

pico_add_extra_outputs(${PROJECT})
pico_enable_stdio_usb(${PROJECT} 0)
pico_enable_stdio_uart(${PROJECT} 1)






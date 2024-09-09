add_library(adctim_lib src/adc_lib.c)

target_include_directories(adctim_lib PUBLIC include)

target_link_libraries(adctim_lib pico_stdlib hardware_adc hardware_timer free_rtos_lib pingpong)
add_library(libadc src/adc_lib.c)

target_include_directories(libadc PUBLIC include)

target_link_libraries(libadc pico_stdlib hardware_adc hardware_timer app_tasks)
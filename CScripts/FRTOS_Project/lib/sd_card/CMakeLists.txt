add_library(sd_lib src/sd_card.c src/sd_task.c)

target_include_directories(sd_lib PUBLIC include)

target_link_libraries(sd_lib free_rtos_lib hardware_spi pingpong_lib)

add_library(app_lib src/app.c)

target_include_directories(app_lib PUBLIC include)

target_link_libraries(app_lib free_rtos_lib tcp_client_lib mpu6050_lib pingpong FatFs_SPI)

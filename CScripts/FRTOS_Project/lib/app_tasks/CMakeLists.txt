add_library(app_tasks src/app_tasks.c)

target_include_directories(app_tasks PUBLIC include)

target_link_libraries(app_tasks FreeRTOS-Kernel-Heap4 tcp_client_lib mpu6050_lib pingpong)

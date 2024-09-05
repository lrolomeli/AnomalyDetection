add_library(mpu6050_lib src/mpu6050.c)
		
target_include_directories(mpu6050_lib PUBLIC include)

target_link_libraries(mpu6050_lib pico_stdlib hardware_i2c)
		



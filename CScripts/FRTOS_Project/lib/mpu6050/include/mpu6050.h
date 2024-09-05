#ifndef MPU6050_H
#define MPU6050_H

#include <stdio.h>

// Function to initialize MPU6050
uint8_t mpu6050_init();

// Function to read accelerometer values (x, y, z)
void mpu6050_read_accel(int16_t* accel_x, int16_t* accel_y, int16_t* accel_z);

// Function to read gyroscope values (x, y, z)
void mpu6050_read_gyro(int16_t* gyro_x, int16_t* gyro_y, int16_t* gyro_z);

#endif /*MPU6050_H*/
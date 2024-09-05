#include "mpu6050.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
#define I2C_PORT i2c0
#define MPU6050_ADDR 0x68
#define I2C_SDA_PIN 0  // Define the SDA pin for I2C
#define I2C_SCL_PIN 1  // Define the SCL pin for I2C

// MPU6050 register addresses
#define REG_PWR_MGMT_1 0x6B
#define REG_ACCEL_XOUT_H 0x3B
#define REG_GYRO_CONFIG 0x1B
#define REG_ACCEL_CONFIG 0x1C
#define REG_SMPLRT_DIV 0x19
#define WHO_AM_I_REG 0x75

// Corresponding configuration values
#define ACCEL_CONFIG_VALUE 0x08  // for ±4g
#define GYRO_CONFIG_VALUE 0x00  // for ±250 degrees per second


// Helper function to read a specific register from the MPU6050
static int8_t mpu6050_read_register(i2c_inst_t* i2c_port, uint8_t reg, uint8_t* data, size_t length) 
{
    return i2c_write_blocking(i2c_port, MPU6050_ADDR, &reg, 1, true) + 
           i2c_read_blocking(i2c_port, MPU6050_ADDR, data, length, false);
}

// Function to initialize MPU6050
uint8_t mpu6050_start() {
	uint8_t reset[] = {REG_PWR_MGMT_1, 0x80};
	uint8_t wake[] = {REG_PWR_MGMT_1, 0x00};
	// Set accelerometer range
    uint8_t accel_config[] = {REG_ACCEL_CONFIG, ACCEL_CONFIG_VALUE};
	
    if (i2c_write_blocking(I2C_PORT, MPU6050_ADDR, reset, 2, false) == PICO_ERROR_GENERIC) {
        return false; // Error writing
    }
	sleep_ms(200);
    if (i2c_write_blocking(I2C_PORT, MPU6050_ADDR, wake, 2, false) == PICO_ERROR_GENERIC) {
        return false; // Error writing
    }
	sleep_ms(200);
    if (i2c_write_blocking(I2C_PORT, MPU6050_ADDR, accel_config, 2, false) == PICO_ERROR_GENERIC) {
        return false; // Error writing
    }
	
    return true; // Successful initialization
}

// Function to read accelerometer values
void mpu6050_read_accel(int16_t* accel_x, int16_t* accel_y, int16_t* accel_z) {
    uint8_t buffer[6];
    mpu6050_read_register(I2C_PORT, REG_ACCEL_XOUT_H, buffer, 6);

    // Convert high and low bytes to 16-bit integers
    *accel_x = (buffer[0] << 8) | buffer[1];
    *accel_y = (buffer[2] << 8) | buffer[3];
    *accel_z = (buffer[4] << 8) | buffer[5];
}

// Function to read gyroscope values
void mpu6050_read_gyro(int16_t* gyro_x, int16_t* gyro_y, int16_t* gyro_z) {
    uint8_t buffer[6];
    mpu6050_read_register(I2C_PORT, REG_GYRO_CONFIG, buffer, 6);

    // Convert high and low bytes to 16-bit integers
    *gyro_x = (buffer[0] << 8) | buffer[1];
    *gyro_y = (buffer[2] << 8) | buffer[3];
    *gyro_z = (buffer[4] << 8) | buffer[5];
}

uint8_t mpu6050_init() 
{
    // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000);
	// i2c_init(I2C_PORT, 100 * 1000);
    // Setup I2C properly
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    // Don't forget the pull ups! | Or use external ones
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Initialize MPU6050
    if (!mpu6050_start()) {
        printf("MPU6050 initialization failed!\n");
        return 1;
    }

    printf("MPU6050 initialized successfully.\n");
	return 0;
	
}
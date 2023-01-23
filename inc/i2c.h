/*
 * Copyright (C) 2020 Bosch Sensortec GmbH
 *
 * The license is available at root folder
 *
 */

/*!
 * @ingroup bme280GroupExample
 * @defgroup bme280GroupExample linux_userspace
 * @brief Linux userspace test code, simple and mose code directly from the doco.
 * compile like this: gcc linux_userspace.c ../bme280.c -I ../ -o bme280
 * tested: Raspberry Pi.
 * Use like: ./bme280 /dev/i2c-0
 * \include linux_userspace.c
 */

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

/******************************************************************************/
/*!                         System header files                               */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

/******************************************************************************/
/*!                         Own header files                                  */
#include "bme280.h"
#include "astruct.h"


/*!
 * @brief This function starts execution of the program.
 */
int initialize_i2c(struct bme280_dev *dev);

/*!
 * @brief This function reading the sensor's registers through I2C bus.
 */
int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len);

/*!
 * @brief This function provides the delay for required time (Microseconds) as per the input provided in some of the
 * APIs
 */
void user_delay_ms(uint32_t period);

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len);

/*!
 * @brief This API used to print the sensor temperature, pressure and humidity data.
 */
void print_sensor_data(struct bme280_data *comp_data);

uint32_t initialize_bme280(struct bme280_dev *dev);

/*!
 * @brief This API reads the sensor temperature, pressure and humidity data in forced mode.
 */
double stream_sensor_data_forced_mode(struct bme280_dev *dev, uint32_t req_delay);

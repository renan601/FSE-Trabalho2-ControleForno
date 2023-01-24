#include <stdio.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include <string.h>
#include <linux/i2c-dev.h> //Used for I2C
#include <sys/ioctl.h>     //Used for I2C
#include <stdlib.h>
#include "../inc/crc16.h"
#include "../inc/bme280.h"
#include "../inc/i2c.h"

/*!
 *  @brief Function for reading the sensor's registers through I2C bus.
 *
 *  @param[in] reg_addr       : Register address.
 *  @param[out] data          : Pointer to the data buffer to store the read data.
 *  @param[in] len            : No of bytes to read.
 *  @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs.
 *
 *  @return Status of execution
 *
 *  @retval 0 -> Success
 *  @retval > 0 -> Failure Info
 *
 */
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    write(id.fd, &reg_addr, 1);
    read(id.fd, data, len);

    return 0;
}

/*!
 * @brief This function provides the delay for required time (Microseconds) as per the input provided in some of the
 * APIs
 */
void user_delay_us(uint32_t period, void *intf_ptr)
{
    usleep(period);
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
    uint8_t *buf;
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    if (write(id.fd, buf, len + 1) < (uint16_t)len)
    {
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}

/*!
 * @brief This API used to print the sensor temperature, pressure and humidity data.
 */
void print_sensor_data(struct bme280_data *comp_data)
{
    float temp, press, hum;

    #ifdef BME280_FLOAT_ENABLE
        temp = comp_data->temperature;
        press = 0.01 * comp_data->pressure;
        hum = comp_data->humidity;
    #else
    #ifdef BME280_64BIT_ENABLE
        temp = 0.01f * comp_data->temperature;
        press = 0.0001f * comp_data->pressure;
        hum = 1.0f / 1024.0f * comp_data->humidity;
    #else
        temp = 0.01f * comp_data->temperature;
        press = 0.01f * comp_data->pressure;
        hum = 1.0f / 1024.0f * comp_data->humidity;
    #endif
    #endif
    printf("%0.2lf deg C, %0.2lf hPa, %0.2lf%%\n", temp, press, hum);
}

/*!
 * @brief This function mounts i2c bus structure
 */
void monta_i2c(struct bme280_dev *dev, struct identifier *id)
{
    /* Make sure to select BME280_I2C_ADDR_PRIM or BME280_I2C_ADDR_SEC as needed */
    id->dev_addr = BME280_I2C_ADDR_PRIM; 
    dev->intf = BME280_I2C_INTF;
    dev->read = user_i2c_read;
    dev->write = user_i2c_write;
    dev->delay_us = user_delay_us;
}

/*!
 * @brief This function opens i2c file to transfer data
 */
void abre_i2c(struct bme280_dev *dev, struct identifier *id)
{
    if ((id->fd = open("/dev/i2c-1", O_RDWR)) < 0)
    {
        fprintf(stderr, "Failed to open the i2c bus %s\n", "/dev/i2c-1");
        exit(1);
    }

    if (ioctl(id->fd, I2C_SLAVE, id->dev_addr) < 0)
    {
        fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    /* Update interface pointer with the structure that contains both device address and file descriptor */
    dev->intf_ptr = id; 
}

/*!
 * @brief This function start the read of the I2C connection
 */
void inicializa_bme280_i2c(struct bme280_dev *dev)
{
    /* Variable to define the result */
    int8_t rslt = BME280_OK; 

    /* Initialize the bme280 */
    rslt = bme280_init(dev); 
    
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", rslt);
        exit(1);
    }
}

/*!
 * @brief This function start the read of the I2C connection
 */
void configura_bme280_i2c(struct bme280_dev *dev)
{
    /* Variable to define the result */
    int8_t rslt = BME280_OK; 

    /* Variable to define the selecting sensors */
    uint8_t settings_sel = 0;

    /* Recommended mode of operation: Indoor navigation */
    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    /* Set the sensor settings */
    rslt = bme280_set_sensor_settings(settings_sel, dev);
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Failed to set sensor settings (code %+d).", rslt);
    }
}

/*!
 * @brief This function start the read of the I2C connection
 */
double le_temp_bme280_i2c(struct bme280_dev *dev)
{
    double temp_amb;

    /* Variable to define the result */
    int8_t rslt = BME280_OK;

    /* Structure to get the pressure, temperature and humidity values */
    struct bme280_data comp_data; 

    /* Variable to store minimum wait time between consecutive measurement in force mode */
    uint32_t req_delay;

    /*Calculate the minimum delay required between consecutive measurement based upon the sensor enabled
    *  and the oversampling configuration. */
    req_delay = bme280_cal_meas_delay(&dev->settings);

    /* Continuously stream sensor data */

    /* Set the sensor to forced mode */
    rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Failed to set sensor mode (code %+d).", rslt);
    }

    /* Wait for the measurement to complete and print data */
    dev->delay_us(req_delay, dev->intf_ptr);
    usleep(100000);
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
    
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Failed to get sensor data (code %+d).", rslt);
    }
    temp_amb = comp_data.temperature;
    // print_sensor_data(&comp_data);
    return temp_amb;
}
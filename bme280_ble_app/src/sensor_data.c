/**
 * @file sensor_data.c
 *
 * Description
 *
 */

/*======= Includes ==========================================================*/

#include <zephyr/types.h>
#include <logging/log.h>

#include <bluetooth/gatt.h>

#include <drivers/sensor.h>

LOG_MODULE_REGISTER(sensor_updater);

/*======= Local Macro Definitions ===========================================*/
/*======= Type Definitions ==================================================*/
/*======= Local function prototypes =========================================*/
/*======= Local variable declarations =======================================*/

extern bool notify_humidity;
extern bool notify_temperature;
extern struct bt_conn *conn;
extern struct bt_gatt_service_static weather_station_svc[];

s32_t pressure = 0;
s32_t temperature = 0;
s32_t humidity = 0;

/*======= Thread prototypes =================================================*/
/*======= Kernal Services Initialization ====================================*/
/*======= Global function implementations ===================================*/

void update_sensor_data(struct device *dev, struct bt_conn *conn)
{
    struct sensor_value temperature_sample, pressure_sample, humidity_sample;
    int err;

    err = sensor_sample_fetch(dev);
    if (0 != err)
    {
        LOG_ERR("Failed to sample sensor data");
        return;
    }
    err = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP,
                             &temperature_sample);
    if (0 != err)
    {
        LOG_ERR("Failed to get sensor data from temperature channel");
    }
    err = sensor_channel_get(dev, SENSOR_CHAN_PRESS, &pressure_sample);
    if (0 != err)
    {
        LOG_ERR("Failed to get sesnor data from pressure channel");
    }
    err = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity_sample);
    if (0 != err)
    {
        LOG_ERR("Failed to get sensor data from humidity channel");
    }

    pressure = pressure_sample.val1;
    temperature = temperature_sample.val1;
    humidity = humidity_sample.val1;
    LOG_INF("temp: %d.%06d; press: %d.%06d; humidity: %d.%06d",
            temperature_sample.val1, temperature_sample.val2,
            pressure_sample.val1, pressure_sample.val2, humidity_sample.val1,
            humidity_sample.val2);

    if ( !conn )
    {
        return;
    }

    if (notify_humidity)
    {
        err = bt_gatt_notify(NULL, &weather_station_svc->attrs[1], &humidity,
                             sizeof(humidity));
        if (0 != err)
        {
            LOG_ERR("Failed to notify the humidity charecterstic in WS service");
        }
    }

    if (notify_temperature)
    {
        err = bt_gatt_notify(NULL, &weather_station_svc->attrs[4], &temperature,
                             sizeof(temperature));
        if (0 != err)
        {
            LOG_ERR("Failed to notify the temperature charecteristic in WS service");
        }
    }
}

/*======= Local function implementations ====================================*/
/*======= Thread implementations ============================================*/

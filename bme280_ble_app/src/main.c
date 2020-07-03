/**
 * @file main.c
 *
 * Description
 *
 */

/*======= Includes ==========================================================*/

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <logging/log.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "sensor_data.h"

LOG_MODULE_REGISTER(main);

/*======= Local Macro Definitions ===========================================*/

#define BME280 DT_INST(0, bosch_bme280)

#if DT_NODE_HAS_STATUS(BME280, okay)
#define BME280_LABEL DT_LABEL(BME280)
#else
#error Your devicetree has no enabled nodes with compatible "bosch,bme280"
#define BME280_LABEL "<none>"
#endif

#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#if DT_PHA_HAS_CELL(LED0_NODE, gpios, flags)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#endif
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN	0
#endif

#ifndef FLAGS
#define FLAGS	0
#endif

/*======= Type Definitions ==================================================*/
/*======= Local function prototypes =========================================*/

static ssize_t recvHumidity(struct bt_conn *conn,
					const struct bt_gatt_attr *attr,
					void *buf, u16_t len,
					u16_t offset);

static ssize_t recvTemperature(struct bt_conn *conn,
					const struct bt_gatt_attr *attr,
					void *buf, u16_t len,
					u16_t offset);

static ssize_t recvPressure(struct bt_conn *conn,
					const struct bt_gatt_attr *attr,
					void *buf, u16_t len,
					u16_t offset);

static void humidity_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value);
static void temperature_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value);

static void bt_ready(int err);
static void connected(struct bt_conn *connected, u8_t err);
static void disconnected(struct bt_conn *disconn, u8_t reason);

/*======= Local variable declarations =======================================*/

extern s32_t pressure;
extern s32_t temperature;
extern s32_t humidity;

/* WS Custom Service  */
static struct bt_uuid_128 ws_service_uuid = BT_UUID_INIT_128(
	0x8f, 0xe5, 0xb3, 0xd5, 0x2e, 0x7f, 0x4a, 0x98,
	0x2a, 0x48, 0x7a, 0xcc, 0x40, 0xfe, 0x00, 0x00);

/* WS Humidity charecterstic */
static struct bt_uuid_128 humidty_char_uuid = BT_UUID_INIT_128(
	0x19, 0xed, 0x82, 0xae, 0xed, 0x21, 0x4c, 0x9d,
	0x41, 0x45, 0x22, 0x8e, 0x41, 0xfe, 0x00, 0x00);

/* WS Temperature charecterstic */
static struct bt_uuid_128 temperature_char_uuid = BT_UUID_INIT_128(
	0x19, 0xed, 0x82, 0xae, 0xed, 0x21, 0x4c, 0x9d,
	0x41, 0x45, 0x22, 0x8e, 0x42, 0xfe, 0x00, 0x00);

/* WS Temperature charecterstic */
static struct bt_uuid_128 pressure_char_uuid = BT_UUID_INIT_128(
	0x19, 0xed, 0x82, 0xae, 0xed, 0x21, 0x4c, 0x9d,
	0x41, 0x45, 0x22, 0x8e, 0x43, 0xfe, 0x00, 0x00);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define ADV_LEN 12

/* Advertising data */
static u8_t manuf_data[ADV_LEN] = {
	0x01 /*SKD version */,
	0x83 /* STM32WB - P2P Server 1 */,
	0x00 /* GROUP A Feature  */,
	0x00 /* GROUP A Feature */,
	0x00 /* GROUP B Feature */,
	0x00 /* GROUP B Feature */,
	0x00, /* BLE MAC start -MSB */
	0x00,
	0x00,
	0x00,
	0x00,
	0x00, /* BLE MAC stop */
};

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, manuf_data, ADV_LEN)
};

/* BLE connection */
struct bt_conn *conn;
/* Notification state */
bool notify_humidity = false;
bool notify_temperature = false;

BT_GATT_SERVICE_DEFINE(weather_station_svc,
BT_GATT_PRIMARY_SERVICE(&ws_service_uuid),
BT_GATT_CHARACTERISTIC(&humidty_char_uuid.uuid,
		       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		       BT_GATT_PERM_READ, recvHumidity, NULL, (void *)1),
BT_GATT_CCC(humidity_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
BT_GATT_CHARACTERISTIC(&temperature_char_uuid.uuid,
		       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			   BT_GATT_PERM_READ, recvTemperature, NULL, (void *)1),
BT_GATT_CCC(temperature_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
BT_GATT_CHARACTERISTIC(&pressure_char_uuid.uuid,
		       BT_GATT_CHRC_READ,
			   BT_GATT_PERM_READ, recvPressure, NULL, &pressure),
);

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

/*======= Thread prototypes =================================================*/
/*======= Kernal Services Initialization ====================================*/
/*======= Global function implementations ===================================*/
/*======= Local function implementations ====================================*/

static ssize_t recvHumidity(struct bt_conn *conn,
					const struct bt_gatt_attr *attr,
					void *buf, u16_t len,
					u16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &humidity,
				 			 sizeof(humidity));
}

static ssize_t recvTemperature(struct bt_conn *conn,
					const struct bt_gatt_attr *attr,
					void *buf, u16_t len,
					u16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &temperature,
				 			 sizeof(temperature));
}

static ssize_t recvPressure(struct bt_conn *conn,
					const struct bt_gatt_attr *attr,
					void *buf, u16_t len,
					u16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &pressure,
				 			 sizeof(pressure));
}

static void humidity_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value)
{
	ARG_UNUSED(attr);
	notify_humidity = (value == BT_GATT_CCC_NOTIFY);
	LOG_INF("Notification %s", notify_humidity ? "enabled" : "disabled");
}

static void temperature_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value)
{
	ARG_UNUSED(attr);
	notify_temperature = (value == BT_GATT_CCC_NOTIFY);
	LOG_INF("Notification %s", notify_temperature ? "enabled" : "disabled");
}

static void bt_ready(int err)
{
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return;
	}
	LOG_INF("Bluetooth initialized");
	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return;
	}

	LOG_INF("Configuration mode: waiting connections...");
}

static void connected(struct bt_conn *connected, u8_t err)
{
	if (err) {
		LOG_ERR("Connection failed (err %u)", err);
	} else {
		LOG_INF("Connected");
		if (!conn) {
			conn = bt_conn_ref(connected);
		}
	}
}

static void disconnected(struct bt_conn *disconn, u8_t reason)
{
	if (conn) {
		bt_conn_unref(conn);
		conn = NULL;
	}

	LOG_INF("Disconnected (reason %u)", reason);
}

/*======= Thread implementations ============================================*/

void main(void)
{
	struct device *sensorDevice = device_get_binding(BME280_LABEL);
	struct device *ledDevice = device_get_binding(LED0);

	if (sensorDevice == NULL) {
		printk("Could not get BME280 device\n");
		return;
	}

	gpio_pin_configure(ledDevice, PIN, GPIO_OUTPUT_INACTIVE | FLAGS );

	printk("dev %p name %s\n", sensorDevice, sensorDevice->name);


	bt_conn_cb_register(&conn_callbacks);

	/* Initialize the Bluetooth Subsystem */
	int err = bt_enable(bt_ready);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
	}

	update_sensor_data(sensorDevice, conn);
	while (1) {
		if (conn)
		{
			gpio_pin_toggle(ledDevice, PIN);
			update_sensor_data(sensorDevice, conn);
			gpio_pin_toggle(ledDevice, PIN);
		}


		k_sleep(K_MSEC(2500));
	}
}

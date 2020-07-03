#ifndef _sensor_data_H_
#define _sensor_data_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file sensor_data.h
 *
 * Description
 *
 */

/*======= Includes ==========================================================*/
/*======= Public macro definitions ==========================================*/
/*======= Type Definitions and declarations =================================*/
/*======= Public variable declarations ======================================*/
/*======= Public function declarations ======================================*/

void update_sensor_data( struct device *dev, struct bt_conn *conn);

#ifdef __cplusplus
}
#endif

#endif /* _sensor_data_H_ */

#ifndef _IMU_UUIDS_H
#define _IMU_UUIDS_H

#include <stdint.h>
#include "imu_ble.h"
#include "../ble_lib_common.h"

#define IMU_SERVICE_UUID            0x180D // Heart rate service UUID
#define IMU_SERVICE_UUID_TYPE     (g_imu_uuid_type)
#define IMU_SVC_RX_CHAR_UUID        0x2A37 // UART RX char UUID
#define IMU_SVC_RX_CHAR_UUID_TYPE (g_imu_uuid_type)
#define IMU_SVC_TX_CHAR_UUID        0x0002// UART TX char UUID
#define IMU_SVC_TX_CHAR_UUID_TYPE (g_imu_uuid_type)

uint32_t imu_uuids_init(ble_dev_c_t * p_ble_imu_c, ble_dev_c_t * p_ble_host_imu_c, ble_dev_c_init_t * p_ble_imu_c_init);

#endif 
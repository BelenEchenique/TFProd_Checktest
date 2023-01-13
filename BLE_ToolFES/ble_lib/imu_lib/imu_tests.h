#ifndef _IMU_TESTS_H_
#define _IMU_TESTS_H_

#include "stdio.h"
#include "nrf_delay.h"
#include "app_uart.h"
#include "nrf_error.h"
#include "imu_ble.h"
#include "string.h"
#include "app_pwm.h"
#include "../ble_lib_common.h"

extern ble_dev_c_t m_ble_dev_c;
// extern conn_peer_t   m_connected_peers[NRF_SDH_BLE_TOTAL_LINK_COUNT];
// extern uint8_t test_client; 
extern stream_mode_t streaming; 
extern stream_mode_t flag_send_streaming; 

#ifndef PWM_PIN1
/* PWM parameters */ 
#define PWM_PIN1                20
#define PWM_PIN2                18
#endif

// Sending bytes 
#define SENSOR_BIN_PREFIX               0X81
#define MODE_SET_SENSOR                 0x80
#define START_STREAM_SENSOR             0x84
#define STOP_STREAM_SENSOR              0x85
#define STREAM_CONFIG_SENSOR            0x8B
#define BATTERY_GET_SENSOR              0x8C
#define STREAM_MEASUREMENT_ACCEL        0X0D
#define STREAM_MEASUREMENT_EULER        0x0E
#define STREAM_MEASUREMENT_GYRO         0x11
#define STREAM_MEASUREMENT_QUAT         0x12
#define STREAM_MEASUREMENT_GAIT_ANGLES  0x13
#define BATTERY_RESP                    0x16
#define STREAM_MEASUREMENT_EXTENDED     0x17
#define STREAM_SENSOR                   0x05
#define STREAMING_TYPE_ACCEL            0X00
#define STREAMING_TYPE_EULER            0x01
#define STREAMING_TYPE_GYR              0X02
#define STREAMING_TYPE_QUAT             0X03
#define STREAMING_TYPE_EXTENDED         0X07
#define SENSOR_COLOR_LED                0x2A

// #define _R     {0x5F, 0x52} 
// #define _N     {0xF5, 0x0A} 
// #define _Mo_3  {0x5F, 0x4D, 0x6F, 0x3B, 0x33}
// #define _Mc_1  {0x2D, 0x4D, 0x63, 0x3B, 0x31}
// #define _T_05_0 {0x5F, 0x54, 0x3B, 0x30, 0x3B, 0x35, 0x3B, 0x30}
// #define 

#ifndef NRF_WriteString
#define NRF_WriteString(command)  {uint8_t * aux = (uint8_t *)command; while(*aux != '\0'){while (app_uart_put(*aux) !=NRF_SUCCESS); aux++;}}
#define NRF_WriteChar(command)    {app_uart_put((uint8_t) command);}
#endif

#define COMMAND_IMU_BLE(command)    ({while (ble_imu_c_string_send(&m_ble_dev_c, (uint8_t *)command, strlen(command)) != NRF_SUCCESS); nrf_delay_ms(170);})
// #define COMMAND_IMU_MULTI(command)    ({while (ble_imu_c_string_send_multilink(&m_ble_dev_c, &m_connected_peers[test_client], (uint8_t *)command, strlen(command)) != NRF_SUCCESS); nrf_delay_ms(170);})
#define HEX_IMU_BLE(command, len)    ({while (ble_imu_c_hex_send(&m_ble_dev_c, (uint8_t *)command, len) != NRF_SUCCESS); nrf_delay_ms(170);})
// #define HEX_IMU_MULTI(command, len)    ({while (ble_imu_c_hex_send(&m_ble_dev_c, &m_connected_peers[test_client], (uint8_t *)command, len) != NRF_SUCCESS); nrf_delay_ms(170);})

/** @brief Commands for serialNumber */
void otaUpdate(volatile state_test_t * state_test);

/** @brief Commands for clearTest */
void colorTest(volatile state_test_t * state_test);

/** @brief Commands for batteryTest */
void batteryTest(volatile state_test_t * state_test, uint8_t rep);

/** @brief Commands for endStreamMode */
void startStreamMode(volatile state_test_t * state_test);

/** @brief Commands for startStreamMode */
void endStreamMode(volatile state_test_t * state_test);

/** @brief Commands for resetTest */
void resetTest(volatile state_test_t * state_test);

/** @brief Commands for infoTest */
void infoTest(volatile state_test_t * state_test);

/** @brief Commands for startStreamMode */
// int decodeSensorStreamedData(uint8_t received_data);
int decodeSensorStreamedData(volatile state_test_t * state_test, uint8_t received_data, sensor_data_t * data_sensor_array, uint8_t counter);

/** @brief Commands for kineticTest */
void kineticTest(volatile state_test_t * state_test, app_pwm_t const * const PWM);

/** @brief Commands for autonomyTest */
void autonomyTest(volatile state_test_t * state_test, app_pwm_t const * const PWM);

#endif
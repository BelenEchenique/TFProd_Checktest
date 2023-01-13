#include "imu_tests.h"
#include "imu_common.h"
#include "string.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "app_pwm.h"
#include "nrf_error.h"

// If new test is added: 
//     - add test to imu_tests.c
//     - add test to imu_tests.h
//     - add state_test_t to imu_commons.h
//     - add state to buffer_process in ble_fsm.h
//     - add state to test_process in main.c

void otaUpdate(volatile state_test_t * state_test){
    
    NRF_WriteString("BLE: Otaupdate\r\n");

    // Mensaje: "-otaupdate";
    // uint8_t send[10] = {0x2d, 0x6f, 0x74, 0x61, 0x75, 0x70, 0x64, 0x61, 0x74, 0x65}; 
    // HEX_IMU_BLE(send, 10);
    *state_test = NONE;
}

void colorTest(volatile state_test_t * state_test){
    if (streaming == ON){
        endStreamMode(state_test); 
        nrf_delay_ms(10); 
        
        // Mensaje: "-batt"
        uint8_t send[6] = {0x2d, 0x63, 0x6c, 0x72, 0x3b, 0x30}; 
        HEX_IMU_BLE(send, 6);
        startStreamMode(state_test);
    }
    else {
        // Mensaje: "-batt"
        uint8_t send[6] = {0x2d, 0x63, 0x6c, 0x72, 0x3b, 0x30}; 
        HEX_IMU_BLE(send, 6);
    }
    *state_test = NONE;
}

void batteryTest(volatile state_test_t * state_test, uint8_t rep){
    // NRF_WriteString("BLE: Battery test\r\n");
    // if (streaming == ON){
    //     endStreamMode(state_test); 
    //     nrf_delay_ms(10); 
        
    //     // Mensaje: "-batt"
    //     for(int i=0; i < rep; i++){
    //         uint8_t send[2] = {0x81, 0x8C}; 
    //         HEX_IMU_BLE(send, 2);
    //         nrf_delay_ms(10); 
    //     }
    //     startStreamMode(state_test);
    // }
    // else {
        // Mensaje: "-batt"
        for(int i=0; i < rep; i++){
            uint8_t send[2] = {0x81, 0x8C}; 
            HEX_IMU_BLE(send, 2);
        }

    // }
    *state_test = NONE;
}

void resetTest(volatile state_test_t * state_test ){
    // NRF_WriteString("BLE: Reset\r\n");

    // Mensaje: "-reset";
    uint8_t send[7] = {0x3e, 0x72, 0x65, 0x73, 0x65, 0x74, 0x0A}; 
    HEX_IMU_BLE(send, 7);
    *state_test = NONE;
}

void infoTest(volatile state_test_t * state_test ){
    NRF_WriteString("BLE: Info test\r\n");

    // Mensaje: "-info";
    // uint8_t send[5] = {0x2d, 0x69, 0x6E,0x66,0x6F}; 
    // HEX_IMU_BLE(send, 5);
    // char buff[6] = "-info";

    COMMAND_IMU_BLE("-info");
    *state_test = NONE;
}

void kineticTest(volatile state_test_t * state_test, app_pwm_t const * const PWM){
    NRF_WriteString("BLE: Kinetic test\r\n");
    uint32_t value;
    // value = 10;

    for (uint8_t i = 0; i < 20; ++i)
    {
        value = (i < 10) ? (i * 10) : (100 - (i - 10) * 10);

        /* Set the duty cycle - keep trying until PWM is ready... */
        while (app_pwm_channel_duty_set(PWM, 0, value) == NRF_ERROR_BUSY);

        nrf_delay_ms(1000);
    }
    /* Set PWM to 0... */
    nrf_gpio_pin_clear(PWM_PIN1);
    nrf_gpio_pin_clear(PWM_PIN2);

    *state_test = NONE;
}

void autonomyTest(volatile state_test_t * state_test, app_pwm_t const * const PWM){
    NRF_WriteString("BLE: Kinetic test\r\n");
    uint32_t value;
    /* Set the duty cycle - keep trying until PWM is ready... */
    nrf_gpio_pin_clear(PWM_PIN1);
    nrf_gpio_pin_clear(PWM_PIN2);

    nrf_delay_ms(1000);
    *state_test = NONE;
}

void startStreamMode(volatile state_test_t * state_test){
    
    NRF_WriteString("BLE: Start stream mode\r\n");
    streaming = ON; 

    uint8_t send_set[3] = {SENSOR_BIN_PREFIX, MODE_SET_SENSOR, STREAM_SENSOR}; 
    HEX_IMU_BLE(send_set, 3);

    // Set modo quats
    // Qué recibo: 
    //     typedef struct {
    //         fix16_t a; // Real part
    //         fix16_t b; // i
    //         fix16_t c; // j
    //         fix16_t d; // k
    //     } qf16;

    uint8_t send_sequence[3] = {SENSOR_BIN_PREFIX, STREAM_CONFIG_SENSOR, STREAM_MEASUREMENT_QUAT}; 
    HEX_IMU_BLE(send_sequence, 3);
    // uint8_t send_sequence[5] = {SENSOR_BIN_PREFIX, STREAM_CONFIG_SENSOR, STREAMING_TYPE_EXTENDED, STREAMING_TYPE_GYR, STREAMING_TYPE_ACCEL}; 
    // HEX_IMU_BLE(send_sequence, 5);

    uint8_t send_start[2] = {SENSOR_BIN_PREFIX, START_STREAM_SENSOR}; 
    HEX_IMU_BLE(send_start, 2);
    
    nrf_delay_ms(100);
    *state_test = NONE;
}

void endStreamMode(volatile state_test_t * state_test){
    uint8_t send[2] = {SENSOR_BIN_PREFIX, STOP_STREAM_SENSOR}; 
    
    HEX_IMU_BLE(send, 2);
    streaming = OFF; 
    
    NRF_WriteString("BLE: Stop stream mode\r\n");
    *state_test = NONE;
}

int decodeSensorStreamedData(volatile state_test_t * state_test, uint8_t received_data, sensor_data_t * data_sensor_array, uint8_t counter){
    data_sensor_array->data_sensor[data_sensor_array->index_data_type] = received_data;
    char buf[20];    
    int32_t aux_angles[3]; 
    long double aux_quats[4]; 

    // TO DO: ignorar la aceleración y giroscopio    
    if (data_sensor_array->index_data_type == SENSOR_PACKAGES - 1){
        for (int i=0; i<20; i++){
            if (i % 2 != 0 && i > 1 && i < 14){
                data_sensor_array->data_decoded[(int) (i-1)/2 - 1] = (data_sensor_array->data_sensor[i] >= 128 ? data_sensor_array->data_sensor[i] - 256 : data_sensor_array->data_sensor[i])*256 + data_sensor_array->data_sensor[i-1];
            }
            else if (i % 2 != 0 && i > 1){
                int j = (i-1)/2 - 7; 
                aux_angles[j] = (data_sensor_array->data_sensor[i] >= 128 ? data_sensor_array->data_sensor[i] - 256 : data_sensor_array->data_sensor[i])*256 + data_sensor_array->data_sensor[i-1];
                aux_quats[j]  = aux_angles[j]/pow(2, 14);
            }
        }

        aux_quats[3] = pow(aux_quats[0], 2) + pow(aux_quats[1], 2) + pow(aux_quats[2], 2) > 1.0 ? 0.0 :
                        sqrt(1 - pow(aux_quats[0], 2) - pow(aux_quats[1], 2) - pow(aux_quats[2], 2));  
        
        if (counter == 0){
            for (int i=0; i<4; i++){
                data_sensor_array->data_decoded[i+6] = aux_quats[i]; 
                gcvt(data_sensor_array->data_decoded[i+6], 6, buf);
                NRF_WriteString(buf);
                NRF_WriteChar(' ');
            }
        }
    }
    data_sensor_array->index_data_type = data_sensor_array->index_data_type == SENSOR_PACKAGES - 1? 0 : data_sensor_array->index_data_type + 1;
    return 0 ;
    *state_test = NONE;
}

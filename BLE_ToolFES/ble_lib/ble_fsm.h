#ifndef _FES_FSM_H_
#define _FES_FSM_H_

#include <string.h>

#include "fes_lib/fes_tests.h"
#include "ble_lib_common.h"


typedef enum possible_devices{
    FES_DEVICE_SELECTED,
    IMU_DEVICE_SELECTED
} device_t; 

volatile device_t selected_device;  

/* States of Testing */
typedef enum process_type{
    NOTHING,
    TEST,
    CONNECTION,
    CHANGE_SETTINGS_TO_IMU,
    CHANGE_SETTINGS_TO_FES,
    DISCONNECT_DEVICE,
    SET,
    UNKNOWN_COMMAND,
    ERROR_T
} process_type_t;

uint16_t own_atoi(uint8_t * buffer_to_int)
{
    uint8_t * ptr = buffer_to_int;
    uint16_t sum = 0;
    uint8_t mult = 1;

    // ptr to \n
    while (*ptr != '\n')
    {
        ptr++;
    }
    ptr--;

    // until unit
    while (*ptr != TRIGGER_CONNECTION)
    {
        if (*ptr > '0' && *ptr <= '9')
        {
            sum += mult * (*ptr - '0');
            mult = mult * 10;
        }
        else    
        {
            sum = 0;
            break;
        }
        ptr--;
    }

    return sum;

}

process_type_t buffer_process(volatile uint8_t * buffer, volatile state_connection_dev_t * state_dev, 
    volatile state_test_t * state_test, device_t selected_device)
{
    process_type_t out = NOTHING;

    switch (*buffer){
    case CHANGE_DEVICE_TO_IMU:
        out = CHANGE_SETTINGS_TO_IMU;
        break;
    case CHANGE_DEVICE_TO_FES:
        out = CHANGE_SETTINGS_TO_FES;
        break;
    case DISCONNECT:
        out = DISCONNECT_DEVICE;
        break; 
    case TRIGGER_CONNECTION:
        out = CONNECTION;
        break;
    case HVTEST:
    case SERIALNUM:
    case RESET: 
    case EXITTEST:
        if (*state_dev == CONNECT_FES && *state_test == NONE){
        // if (selected_device == FES_DEVICE_SELECTED && *state_dev == CONNECT_FES && *state_test == NONE){
            *state_test = *buffer;
            out = TEST;
        }
        else{
            out = ERROR_T;
        }
        break;
    case OTAUPDATE:
    case BATTERY:
    case CLR: 
    case INFO: 
    case STARTSTREAM: 
    case ENDSTREAM: 
    case AUTONOMY:
    case KINETICS:
    case END_KINETICS:
    case SEND_STREAMING: 
        if (*state_dev == CONNECT_IMU && *state_test == NONE){
        // if (selected_device == IMU_DEVICE_SELECTED && *state_dev == CONNECT_IMU && *state_test == NONE){
            *state_test = *buffer;
            out = TEST;
        }
        else{
            out = ERROR_T;
        }
        break;
    // case KINETICS:
        // if (selected_device == IMU_DEVICE_SELECTED && *state_test == NONE){
        //     *state_test = *buffer;
        //     out = TEST;
        // }
        // break; 
    default:
        out = UNKNOWN_COMMAND;
        break;
    }
    return out;
}

/**@brief     Function for empty the Clients Buffer.
 *
 * @param[in] Clients     Pointer to Clients Structure.
 */
void emptyClients(clients_t * Clients)
{
    uint8_t last_client = Clients->last_client;
    for (uint8_t i = 0; i < 6 ; i++)
    {
        for (uint8_t j = 0; j < last_client ; j++)
        {
            Clients->buffer[i][j] = 0;
        }
    }
    Clients->last_client = 0;
}

#endif

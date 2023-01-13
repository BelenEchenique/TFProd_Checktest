#ifndef _FES_TESTS_H_
#define _FES_TESTS_H_

#include "stdio.h"
#include "nrf_delay.h"
#include "app_uart.h"
#include "nrf_error.h"
#include "fes_ble.h"
#include "string.h"
#include "../ble_lib_common.h"

extern ble_dev_c_t  m_ble_dev_c;


#define COMMAND_FES_BLE(command)    ({while (ble_fes_c_string_send(&m_ble_dev_c, (uint8_t *)command, strlen(command)) != NRF_SUCCESS); nrf_delay_ms(170);})
#define PRINT_TO_ATSAM(command)   {uint8_t * aux = (uint8_t *)command; while(*aux != '\0'){while (app_uart_put(*aux) !=NRF_SUCCESS); aux++;}}    
#define CHAR_TO_ATSAM(command)    {app_uart_put((uint8_t)command);}

/** @brief Commands for highVoltageTest */
void highVoltageTest(volatile state_test_t * state_test, volatile bool * fes_adv);

/** @brief Commands for serialNumber */
void serialNumber(volatile state_test_t * state_test, volatile bool * fes_adv);

#endif
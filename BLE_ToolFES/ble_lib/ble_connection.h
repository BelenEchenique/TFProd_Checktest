#ifndef _BLE_CONNECTION_H_
#define _BLE_CONNECTION_H_

#include <stdbool.h>
#include "ble.h"
#include "app_util.h"
#include "ble_gap.h"
#include "nrf_error.h"
#include "bsp.h"
#include "ble_gap.h"
#include "ble_advdata.h"
#include "ble_db_discovery.h"
#include "ble_lib_common.h"

void ble_fes_c_on_ble_evt(ble_dev_c_t * p_ble_fes_c, const ble_evt_t * p_ble_evt);

void on_hvx(ble_dev_c_t * p_ble_fes_c, const ble_evt_t * p_ble_evt);

uint32_t adv_report_parse(uint8_t type, uint8_array_t * p_advdata, uint8_array_t * p_typedata);

#endif
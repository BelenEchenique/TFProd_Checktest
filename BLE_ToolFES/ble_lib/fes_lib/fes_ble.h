#ifndef _FES_BLE_H_
#define _FES_BLE_H_

#include <stdbool.h>
#include "ble.h"
#include "fes_uuids.h"
#include "app_util.h"
#include "ble_gap.h"
#include "nrf_error.h"
#include "bsp.h"
#include "ble_gap.h"
#include "ble_advdata.h"
#include "ble_db_discovery.h"
#include "../ble_lib_common.h"
#include "../ble_connection.h"

void ble_fes_c_on_db_disc_evt(ble_dev_c_t * p_ble_fes_c, ble_db_discovery_evt_t * p_evt);

void on_adv_report_fes(const ble_evt_t * const p_ble_evt, clients_t * Clients, const ble_gap_evt_t * p_gap_evt);

bool is_uuid_present(const ble_uuid_t *p_target_uuid, const ble_gap_evt_adv_report_t *p_adv_report);

uint8_t is_client_fes(clients_t * Clients, const ble_gap_evt_adv_report_t * p_adv_report);

uint32_t ble_fes_c_handles_assign(ble_dev_c_t * p_ble_nus,
                                  const uint16_t conn_handle,
                                  const ble_dev_c_handles_t * p_peer_handles);

uint32_t ble_fes_c_rx_notif_enable(ble_dev_c_t * p_ble_fes_c);

uint32_t ble_fes_c_string_send(ble_dev_c_t * p_ble_fes_c, uint8_t * p_string, uint16_t length);

#endif
#ifndef _IMU_BLE_H_
#define _IMU_BLE_H_

#include <stdbool.h>
#include "ble.h"
#include "imu_uuids.h"
#include "app_util.h"
#include "ble_gap.h"
#include "nrf_error.h"
#include "bsp.h"
#include "ble_gap.h"
#include "ble_advdata.h"
#include "ble_db_discovery.h"
#include "../ble_lib_common.h"
#include "../ble_connection.h"

#define RAWDATASIZE 11

extern uint8_t client_counter; 
extern uint8_t test_client; 

void ble_imu_c_on_db_disc_evt(ble_dev_c_t * p_ble_imu_c, ble_db_discovery_evt_t * p_evt);

void on_adv_report_imu(const ble_evt_t * const p_ble_evt, clients_t * Clients, const ble_gap_evt_t * p_gap_evt);

uint8_t find_in_seq(const ble_evt_t * const p_ble_evt);

uint8_t find_gap_adr(const ble_evt_t * const p_ble_evt, uint8_t adv_index);

uint8_t is_client_imu(clients_t * Clients, const ble_gap_evt_adv_report_t * p_adv_report, uint8_t *new_id, uint8_t model);

uint32_t ble_imu_c_handles_assign(ble_dev_c_t * p_ble_nus, const uint16_t conn_handle, const ble_dev_c_handles_t * p_peer_handles);

uint32_t ble_imu_c_rx_notif_enable(ble_dev_c_t * p_ble_imu_c);

uint32_t ble_imu_c_string_send(ble_dev_c_t * p_ble_imu_c, uint8_t * p_string, uint16_t length);

uint32_t ble_imu_c_string_send_multilink(ble_dev_c_t * p_ble_imu_c, conn_peer_t * connected_peers, uint8_t * p_string, uint16_t length);

uint32_t ble_imu_c_hex_send_multilink(ble_dev_c_t * p_ble_imu_c, conn_peer_t * connected_peers, uint8_t * p_string, uint8_t len);

uint32_t ble_imu_c_hex_send(ble_dev_c_t * p_ble_imu_c, uint8_t * p_string, uint8_t length);

void write_client_name(clients_t * Clients, const ble_gap_evt_adv_report_t * p_adv_report, uint8_t current_client);

void write_chosen_imu(chosen_t * imu_chosen);
// void write_chosen_imu(chosen_t * imu_chosen, conn_peer_t * m_connected_peers);

uint8_t search_for_client(clients_t * Clients);

#endif
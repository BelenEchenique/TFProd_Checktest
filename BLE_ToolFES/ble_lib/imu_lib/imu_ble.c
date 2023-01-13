#include "../ble_lib_common.h"
#include "sdk_macros.h"
#include "imu_ble.h"
#include "ble_db_discovery.h"
// #include "imu_common.h"
#include "ble_gattc.h"
#include "imu_uuids.h"
#include "app_uart.h"
#include <stdbool.h>

#ifndef NRF_WriteString
#define NRF_WriteString(command)  {uint8_t * aux = (uint8_t *)command; while(*aux != '\0'){while (app_uart_put(*aux) !=NRF_SUCCESS); aux++;}}
#define NRF_WriteChar(command)    {app_uart_put((uint8_t) command);}
#endif

uint8_t rawdata[RAWDATASIZE] = {0x09, 0x54, 0x72, 0x61, 0x69, 0x6E, 0x46, 0x45, 0x53, 0x20, 0x53}; 
uint8_t new_id[2];
uint8_t model;

void ble_imu_c_on_db_disc_evt(ble_dev_c_t * p_ble_imu_c, ble_db_discovery_evt_t * p_evt)
{

    ble_gatt_db_char_t * p_chars = p_evt->params.discovered_db.charateristics;

    if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE &&
        p_evt->params.discovered_db.srv_uuid.uuid == IMU_SERVICE_UUID &&
        p_evt->params.discovered_db.srv_uuid.type == p_ble_imu_c->uuid_type)
    {

        ble_dev_c_evt_t imu_c_evt;
        imu_c_evt.evt_type    = BLE_DEV_C_EVT_DISCOVERY_COMPLETE;
        imu_c_evt.conn_handle = p_evt->conn_handle;
    
        uint32_t i;
        for (i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {  
            switch (p_chars[i].characteristic.uuid.uuid)
            {
                case IMU_SVC_TX_CHAR_UUID: 
                    imu_c_evt.handles.dev_tx_handle = p_chars[i].characteristic.handle_value;
                    break;
                case IMU_SVC_RX_CHAR_UUID:
                    imu_c_evt.handles.dev_rx_handle = p_chars[i].characteristic.handle_value;
                    imu_c_evt.handles.dev_rx_cccd_handle = p_chars[i].cccd_handle;
                    break;
            }
        }
        if (p_ble_imu_c->evt_handler != NULL)
        {
            imu_c_evt.conn_handle = p_evt->conn_handle;
            imu_c_evt.evt_type    = BLE_DEV_C_EVT_DISCOVERY_COMPLETE;
            p_ble_imu_c->evt_handler(p_ble_imu_c, &imu_c_evt);
        }
    }
}

/**
 * @brief ON Advertisment function for handler
 *
 * @param[in]  Clients      Clients of IMU
 * @param[in]  p_adv_report p_adv_report of IMU  
 */

void on_adv_report_imu(const ble_evt_t * const p_ble_evt, clients_t * Clients, const ble_gap_evt_t * p_gap_evt){

    const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;
    uint8_array_t adv_data;
    uint8_array_t dev_name;
    uint32_t err_code;

    adv_data.p_data = (uint8_t *)p_gap_evt->params.adv_report.data;
    adv_data.size   = p_gap_evt->params.adv_report.dlen;
    
    if (find_in_seq(p_ble_evt))
    {   
        uint8_t id_start = find_gap_adr(p_ble_evt, find_in_seq(p_ble_evt));
         
        uint8_t imu_cases = is_client_imu(Clients, p_adv_report, new_id, model);
        uint8_t current_client = Clients->last_client;

        if(imu_cases == CLIENT_BUFFER_ALLOW)
        {
            err_code = adv_report_parse(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
                            &adv_data,
                            &dev_name);
            if (err_code != NRF_SUCCESS) printf("No long name\r\n");
            else{
                Clients->num_serial[current_client].size = dev_name.size;
                for(uint16_t i = 0 ; i < Clients->num_serial[current_client].size ; i++)
                    Clients->num_serial[current_client].p_data[i] = dev_name.p_data[i];

                for(uint16_t i = 0 ; i < 2 ; i++)
                    Clients->id[i][current_client] = p_ble_evt->evt.gap_evt.params.adv_report.data[id_start + 8 - i];
                Clients->model[current_client]     = p_ble_evt->evt.gap_evt.params.adv_report.data[id_start + 3];
                Clients->peer_addr[current_client] = p_ble_evt->evt.gap_evt.params.connected.peer_addr;
                
                write_client_name(Clients, p_adv_report, current_client);
            }
        }
    }
}

uint8_t find_in_seq(const ble_evt_t * const p_ble_evt){
    int counter = 0;
    for (int i=0; i<sizeof(p_ble_evt->evt.gap_evt.params.adv_report.data); i++){
        if (counter == RAWDATASIZE - 1 && p_ble_evt->evt.gap_evt.params.adv_report.data[i] == rawdata[counter])
        {
            counter = 0;
            return i;
        }
        else if (counter < RAWDATASIZE && p_ble_evt->evt.gap_evt.params.adv_report.data[i] == rawdata[counter]){
            counter++; 
        }
        else if (counter < RAWDATASIZE - 1 && p_ble_evt->evt.gap_evt.params.adv_report.data[i] != rawdata[counter]){
            counter  = 0; 
        }
    }
    return 0;
}

uint8_t find_gap_adr(const ble_evt_t * const p_ble_evt, uint8_t adv_index){
    for (int i=0; i<adv_index; i++){
        if (i < adv_index && p_ble_evt->evt.gap_evt.params.adv_report.data[i] == 0xFF){
            if (p_ble_evt->evt.gap_evt.params.adv_report.data[i+1] == 0xA8 && p_ble_evt->evt.gap_evt.params.adv_report.data[i+2] == 0x86){
                model     = p_ble_evt->evt.gap_evt.params.adv_report.data[i+3];
                new_id[0] = p_ble_evt->evt.gap_evt.params.adv_report.data[i+8];
                new_id[1] = p_ble_evt->evt.gap_evt.params.adv_report.data[i+7];
                return i; 
            }
        }
    }
    return 0;
}

/* Check if imu is in the client list return 0 if can be added, 0x01 if error of complete buffer, 0x02 if client exists */
uint8_t is_client_imu(clients_t * Clients, const ble_gap_evt_adv_report_t * p_adv_report, uint8_t *new_id, uint8_t model)
{
    if (Clients->last_client >= CLIENTS_MAX_SIZE)
    {
        return CLIENT_BUFFER_FULL;
    }

    uint8_t client = 0;
    uint8_t i = 0;

    /* Check if client exists */
    while(client < Clients->last_client)
    {
        if (Clients->id[0][client] == new_id[0] && Clients->id[1][client] == new_id[1] && Clients->model[client] == model)
        {
            return CLIENT_BUFFER_EXISTS;
        }
        else if(i == 1)
        {
            break; 
        }
        client++;
    }
    return CLIENT_BUFFER_ALLOW;
}

/* 
        Char functions. 
*/
uint32_t ble_imu_c_handles_assign(ble_dev_c_t * p_ble_imu, const uint16_t conn_handle, const ble_dev_c_handles_t * p_peer_handles)
{
    VERIFY_PARAM_NOT_NULL(p_ble_imu);

    p_ble_imu->conn_handle = conn_handle;
    if (p_peer_handles != NULL)
    {
        p_ble_imu->handles.dev_rx_cccd_handle = p_peer_handles->dev_rx_cccd_handle;
        p_ble_imu->handles.dev_rx_handle      = p_peer_handles->dev_rx_handle;
        p_ble_imu->handles.dev_tx_handle      = p_peer_handles->dev_tx_handle;
    }
    return NRF_SUCCESS;
}

/**@brief Function for creating a message for writing to the CCCD.
 */
static uint32_t cccd_configure(uint16_t conn_handle, uint16_t cccd_handle, bool enable)
{
    uint8_t buf[BLE_CCCD_VALUE_LEN];

    buf[0] = BLE_GATT_HVX_NOTIFICATION;
    buf[1] = 0;

    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_REQ,
        .handle   = cccd_handle,
        .offset   = 0,
        .len      = sizeof(buf),
        .p_value  = buf
    };

    return sd_ble_gattc_write(conn_handle, &write_params);
}

uint32_t ble_imu_c_rx_notif_enable(ble_dev_c_t * p_ble_imu_c)
{   
    VERIFY_PARAM_NOT_NULL(p_ble_imu_c);

    if ( (p_ble_imu_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       ||(p_ble_imu_c->handles.dev_rx_cccd_handle == BLE_GATT_HANDLE_INVALID)
       )
    {
        return NRF_ERROR_INVALID_STATE;
    }
    return cccd_configure(p_ble_imu_c->conn_handle,p_ble_imu_c->handles.dev_rx_cccd_handle, true);
}

uint32_t ble_imu_c_string_send(ble_dev_c_t * p_ble_imu_c, uint8_t * p_string, uint16_t length)
{
    VERIFY_PARAM_NOT_NULL(p_ble_imu_c);

    if (length > BLE_IMU_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (p_ble_imu_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = p_ble_imu_c->handles.dev_tx_handle,
        .offset   = 0,
        .len      = length,
        .p_value  = p_string
    };

    return sd_ble_gattc_write(p_ble_imu_c->conn_handle, &write_params);
}

uint32_t ble_imu_c_hex_send_multilink(ble_dev_c_t * p_ble_imu_c, conn_peer_t * connected_peers, uint8_t * p_string, uint8_t length)
{
    VERIFY_PARAM_NOT_NULL(p_ble_imu_c);

    if (length > BLE_IMU_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (connected_peers->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (test_client > client_counter){
        NRF_WriteString("Error: No client.\r\n");
        return NRF_SUCCESS;
    }

    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = p_ble_imu_c->handles.dev_tx_handle,
        .offset   = 0,
        .len      = length,
        .p_value  = p_string
    };

    return sd_ble_gattc_write(connected_peers->conn_handle, &write_params);
}

uint32_t ble_imu_c_hex_send(ble_dev_c_t * p_ble_imu_c, uint8_t * p_string, uint8_t length)
{
    VERIFY_PARAM_NOT_NULL(p_ble_imu_c);

    if (length > BLE_IMU_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (p_ble_imu_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = p_ble_imu_c->handles.dev_tx_handle,
        .offset   = 0,
        .len      = length,
        .p_value  = p_string
    };

    return sd_ble_gattc_write(p_ble_imu_c->conn_handle, &write_params);
}

uint32_t ble_imu_c_string_send_multilink(ble_dev_c_t * p_ble_imu_c, conn_peer_t * connected_peers, uint8_t * p_string, uint16_t length)
{
    VERIFY_PARAM_NOT_NULL(p_ble_imu_c);

    if (length > BLE_IMU_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (connected_peers->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (test_client > client_counter){
        NRF_WriteString("Error: No client.\r\n");
        return NRF_SUCCESS;
    }

    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = p_ble_imu_c->handles.dev_tx_handle,
        .offset   = 0,
        .len      = length,
        .p_value  = p_string
    };

    return sd_ble_gattc_write(connected_peers->conn_handle, &write_params);
}

void write_client_name(clients_t * Clients, const ble_gap_evt_adv_report_t * p_adv_report,uint8_t current_client){
    char buf1[10], buf2[10];
    itoa(Clients->id[0][current_client],buf1,10);
    itoa(Clients->id[1][current_client],buf2,10);

    if (Clients->id[0][current_client] < 10 && Clients->id[1][current_client] < 10)
        printf("S0%s0%s ", buf1, buf2);
    else if (Clients->id[0][current_client] < 100 && Clients->id[1][current_client] < 100)
        printf("S0%s%s ", buf1, buf2);
    else // if (Clients->id[1][current_client] < 10){
        printf("S%s%s ", buf1, buf2);

    printf("I%d ", p_adv_report->rssi);
    for(uint8_t k = 0 ; k < 6 ; k++)
    {
        Clients->buffer[k][current_client] = p_adv_report->peer_addr.addr[k];
    }
    Clients->last_client = current_client + 1;
    printf("C%d\r\n", Clients->last_client);
}

void write_chosen_imu(chosen_t * imu_chosen){
// void write_chosen_imu(chosen_t * imu_chosen, conn_peer_t * m_connected_peers){
    char buf1[10], buf2[10], buf3[10];
    itoa(imu_chosen->id[0],buf1,10);
    itoa(imu_chosen->id[1],buf2,10);

    // NRF_WriteChar('S');
    if (imu_chosen->id[0] < 10 && imu_chosen->id[1] < 10){
        sprintf(buf3, "S0%s0", buf1);
    }
    else if (imu_chosen->id[0] < 10 && imu_chosen->id[1] < 100){
        sprintf(buf3, "S0%s", buf1);
    }
    else {
        sprintf(buf3, "S%s", buf1);
    }
    strcat(buf3, buf2);
    // strcpy(m_connected_peers->name, buf3); 
    
    NRF_WriteString(buf3); 
    // NRF_WriteString(m_connected_peers->name); 

}

uint8_t search_for_client(clients_t * Clients){
    for(uint8_t i = 0; i < CLIENTS_MAX_SIZE; i++){
        if (Clients->id[0][i] == 0 && Clients->id[1][i] == 30){
            printf("Cliente: %d\r\n", i+1);
            return i+1;
        }
    }
    return 0; 
}
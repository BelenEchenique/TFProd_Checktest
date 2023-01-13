#include "sdk_macros.h"
#include "fes_ble.h"
#include "ble_db_discovery.h"
#include "fes_common.h"
#include "ble_gattc.h"
#include "fes_uuids.h"
#include "../ble_lib_common.h"

void ble_fes_c_on_db_disc_evt(ble_dev_c_t * p_ble_fes_c, ble_db_discovery_evt_t * p_evt)
{
    ble_dev_c_evt_t fes_c_evt;
    memset(&fes_c_evt,0,sizeof(ble_dev_c_evt_t));

    ble_gatt_db_char_t * p_chars = p_evt->params.discovered_db.charateristics;

    if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE &&
        p_evt->params.discovered_db.srv_uuid.uuid == FES_SERVICE_UUID &&
        p_evt->params.discovered_db.srv_uuid.type == p_ble_fes_c->uuid_type)
    {
        uint32_t i;

        for (i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            switch (p_chars[i].characteristic.uuid.uuid)
            {
                case FES_SVC_TX_CHAR_UUID:
                    fes_c_evt.handles.dev_tx_handle = p_chars[i].characteristic.handle_value;
                    break;

                case FES_SVC_RX_CHAR_UUID:
                    fes_c_evt.handles.dev_rx_handle = p_chars[i].characteristic.handle_value;
                    fes_c_evt.handles.dev_rx_cccd_handle = p_chars[i].cccd_handle;
                    break;

                default:
                    break;
            }
        }
        if (p_ble_fes_c->evt_handler != NULL)
        {
            fes_c_evt.conn_handle = p_evt->conn_handle;
            fes_c_evt.evt_type    = BLE_DEV_C_EVT_DISCOVERY_COMPLETE;
            p_ble_fes_c->evt_handler(p_ble_fes_c, &fes_c_evt);
        }
    }
}

/**
 * @brief ON Advertisment function for handler
 *
 * @param[in]  Clients      Clients of FES
 * @param[in]  p_adv_report p_adv_report of FES  
 */
void on_adv_report_fes(const ble_evt_t * const p_ble_evt, clients_t * Clients, const ble_gap_evt_t * p_gap_evt){
    ble_uuid_t m_fes_uuid =
    {
        .uuid = FES_SERVICE_UUID,
        .type = BLE_UUID_TYPE_BLE
    };

    const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;
    uint8_array_t adv_data;
    uint8_array_t dev_name;
    uint32_t err_code;

    adv_data.p_data = (uint8_t *)p_gap_evt->params.adv_report.data;
    adv_data.size   = p_gap_evt->params.adv_report.dlen;
    
    if (is_uuid_present(&m_fes_uuid, p_adv_report))
    {
        uint8_t fes_cases = is_client_fes(Clients, p_adv_report);
        uint8_t current_client = Clients->last_client;
        if(fes_cases == CLIENT_BUFFER_ALLOW)
        {
            err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
                            &adv_data,
                            &dev_name);
            if (err_code != NRF_SUCCESS) printf("No long name\r\n");
            else{
                Clients->num_serial[current_client].size = dev_name.size;
                for(uint16_t i = 0 ; i < Clients->num_serial[current_client].size ; i++)
                    Clients->num_serial[current_client].p_data[i] = dev_name.p_data[i];
                // printf("S");
                for(uint16_t i = 0 ; i < Clients->num_serial[current_client].size ; i++)
                {
                    printf("%c", Clients->num_serial[current_client].p_data[i]);
                }
                printf(" ");
            }
            printf("I%d ", p_adv_report->rssi);
            for(uint8_t k = 0 ; k < 6 ; k++)
            {
                Clients->buffer[k][current_client] = p_adv_report->peer_addr.addr[k];
            }
            Clients->last_client = current_client + 1;
            printf("C%d\n", Clients->last_client);
        }
    }
}

/**@brief Reads an advertising report and checks if a uuid is present in the service list.
 *
 * @details The function is able to search for 16-bit, 32-bit and 128-bit service uuids.
 *          To see the format of a advertisement packet, see
 *          https://www.bluetooth.org/Technical/AssignedNumbers/generic_access_profile.htm
 *
 * @param[in]   p_target_uuid The uuid to search fir
 * @param[in]   p_adv_report  Pointer to the advertisement report.
 *
 * @retval      true if the UUID is present in the advertisement report. Otherwise false
 */
bool is_uuid_present(const ble_uuid_t *p_target_uuid, const ble_gap_evt_adv_report_t *p_adv_report)
{
    uint32_t err_code;
    uint32_t index = 0;
    uint8_t *p_data = (uint8_t *)p_adv_report->data;
    ble_uuid_t extracted_uuid;

    while (index < p_adv_report->dlen)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if ( (field_type == BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE)
           || (field_type == BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE)
           )
        {
            for (uint32_t u_index = 0; u_index < (field_length / UUID16_SIZE); u_index++)
            {
                err_code = sd_ble_uuid_decode(  UUID16_SIZE,
                                                &p_data[u_index * UUID16_SIZE + index + 2],
                                                &extracted_uuid);
                if (err_code == NRF_SUCCESS)
                {
                    if ((extracted_uuid.uuid == p_target_uuid->uuid)
                        && (extracted_uuid.type == p_target_uuid->type))
                    {
                        return true;
                    }
                }
            }
        }

        else if ( (field_type == BLE_GAP_AD_TYPE_32BIT_SERVICE_UUID_MORE_AVAILABLE)
                || (field_type == BLE_GAP_AD_TYPE_32BIT_SERVICE_UUID_COMPLETE)
                )
        {
            for (uint32_t u_index = 0; u_index < (field_length / UUID32_SIZE); u_index++)
            {
                err_code = sd_ble_uuid_decode(UUID16_SIZE,
                &p_data[u_index * UUID32_SIZE + index + 2],
                &extracted_uuid);
                if (err_code == NRF_SUCCESS)
                {
                    if ((extracted_uuid.uuid == p_target_uuid->uuid)
                        && (extracted_uuid.type == p_target_uuid->type))
                    {
                        return true;
                    }
                }
            }
        }

        else if ( (field_type == BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE)
                || (field_type == BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE)
                )
        {
            err_code = sd_ble_uuid_decode(UUID128_SIZE,
                                          &p_data[index + 2],
                                          &extracted_uuid);
            if (err_code == NRF_SUCCESS)
            {
                if ((extracted_uuid.uuid == p_target_uuid->uuid)
                    && (extracted_uuid.type == p_target_uuid->type))
                {
                    return true;
                }
            }
        }
        index += field_length + 1;
    }
    return false;
}

/* Check if fes is in the client list return 0 if can be added, 0x01 if error of complete buffer, 0x02 if client exists */
uint8_t is_client_fes(clients_t * Clients, const ble_gap_evt_adv_report_t * p_adv_report)
{
    if (Clients->last_client >= CLIENTS_FES_MAX_SIZE)
    {
        return CLIENT_BUFFER_FULL;
    }

    uint8_t client = 0;
    uint8_t i = 0;

    /* Check if client exists */
    while(client < Clients->last_client)
    {
        while(i < 6)
        {
            if (Clients->buffer[i][client] != p_adv_report->peer_addr.addr[i])
            {
                break;
            }
            else if(i == 5)
            {
                return CLIENT_BUFFER_EXISTS;
            }
            i++;
        }
        client++;
    }
    return CLIENT_BUFFER_ALLOW;
}

/* 
        Char functions. 
*/
uint32_t ble_fes_c_handles_assign(ble_dev_c_t * p_ble_fes, const uint16_t conn_handle, const ble_dev_c_handles_t * p_peer_handles)
{
    VERIFY_PARAM_NOT_NULL(p_ble_fes);

    p_ble_fes->conn_handle = conn_handle;
    if (p_peer_handles != NULL)
    {
        p_ble_fes->handles.dev_rx_cccd_handle = p_peer_handles->dev_rx_cccd_handle;
        p_ble_fes->handles.dev_rx_handle      = p_peer_handles->dev_rx_handle;
        p_ble_fes->handles.dev_tx_handle      = p_peer_handles->dev_tx_handle;
    }
    return NRF_SUCCESS;
}

/**@brief Function for creating a message for writing to the CCCD.
 */
static uint32_t cccd_configure(uint16_t conn_handle, uint16_t cccd_handle, bool enable)
{
    uint8_t buf[BLE_CCCD_VALUE_LEN];

    buf[0] = enable ? BLE_GATT_HVX_NOTIFICATION : 0;
    buf[1] = 0;

    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_REQ,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = cccd_handle,
        .offset   = 0,
        .len      = sizeof(buf),
        .p_value  = buf
    };

    return sd_ble_gattc_write(conn_handle, &write_params);
}

uint32_t ble_fes_c_rx_notif_enable(ble_dev_c_t * p_ble_fes_c)
{
    VERIFY_PARAM_NOT_NULL(p_ble_fes_c);

    if ( (p_ble_fes_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       ||(p_ble_fes_c->handles.dev_rx_cccd_handle == BLE_GATT_HANDLE_INVALID)
       )
    {
        return NRF_ERROR_INVALID_STATE;
    }
    return cccd_configure(p_ble_fes_c->conn_handle,p_ble_fes_c->handles.dev_rx_cccd_handle, true);
}

uint32_t ble_fes_c_string_send(ble_dev_c_t * p_ble_fes_c, uint8_t * p_string, uint16_t length)
{
    VERIFY_PARAM_NOT_NULL(p_ble_fes_c);

    if (length > BLE_FES_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (p_ble_fes_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    // sprintf((char *) p_string, "_%s", (char *) p_string);

    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = p_ble_fes_c->handles.dev_tx_handle,
        .offset   = 0,
        .len      = length,
        .p_value  = p_string
    };

    return sd_ble_gattc_write(p_ble_fes_c->conn_handle, &write_params);
}
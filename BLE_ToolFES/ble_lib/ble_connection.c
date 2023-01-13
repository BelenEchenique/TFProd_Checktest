#include "sdk_macros.h"
#include "ble_connection.h"
#include "ble_db_discovery.h"
#include "ble_gattc.h"
#include "ble_lib_common.h"

void ble_fes_c_on_ble_evt(ble_dev_c_t * p_ble_fes_c, const ble_evt_t * p_ble_evt)
{
    if ((p_ble_fes_c == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    if ( (p_ble_fes_c->conn_handle != BLE_CONN_HANDLE_INVALID)
       &&(p_ble_fes_c->conn_handle != p_ble_evt->evt.gap_evt.conn_handle)
       )
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTC_EVT_HVX:
            on_hvx(p_ble_fes_c, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            if (p_ble_evt->evt.gap_evt.conn_handle == p_ble_fes_c->conn_handle
                    && p_ble_fes_c->evt_handler != NULL)
            {
                ble_dev_c_evt_t fes_c_evt;

                fes_c_evt.evt_type = BLE_DEV_C_EVT_DISCONNECTED;

                p_ble_fes_c->conn_handle = BLE_CONN_HANDLE_INVALID;
                p_ble_fes_c->evt_handler(p_ble_fes_c, &fes_c_evt);
            }
            break;
    }
}


/**@brief     Function for handling Handle Value Notification received from the SoftDevice.
 *
 * @details   This function will uses the Handle Value Notification received from the SoftDevice
 *            and checks if it is a notification of the NUS RX characteristic from the peer. If
 *            it is, this function will decode the data and send it to the
 *            application.
 *
 * @param[in] p_ble_fes_c Pointer to the FES Client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event received.
 */
void on_hvx(ble_dev_c_t * p_ble_fes_c, const ble_evt_t * p_ble_evt)
{
    // HVX can only occur from client sending.
    if ( (p_ble_fes_c->handles.dev_rx_handle != BLE_GATT_HANDLE_INVALID)
            && (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_fes_c->handles.dev_rx_handle)
            && (p_ble_fes_c->evt_handler != NULL)
        )
    {
        ble_dev_c_evt_t ble_fes_c_evt;

        ble_fes_c_evt.evt_type = BLE_DEV_C_EVT_DEV_RX_EVT;
        ble_fes_c_evt.p_data   = (uint8_t *)p_ble_evt->evt.gattc_evt.params.hvx.data;
        ble_fes_c_evt.data_len = p_ble_evt->evt.gattc_evt.params.hvx.len;

        p_ble_fes_c->evt_handler(p_ble_fes_c, &ble_fes_c_evt);
    }
}

uint32_t adv_report_parse(uint8_t type, uint8_array_t * p_advdata, uint8_array_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->size)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type)
        {
            p_typedata->p_data = &p_data[index + 2];
            p_typedata->size   = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}

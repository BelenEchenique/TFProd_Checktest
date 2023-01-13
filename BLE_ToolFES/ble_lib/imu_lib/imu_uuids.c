#include "sdk_macros.h"
#include "imu_uuids.h"
#include "app_error.h"
#include "ble.h"
#include "ble_db_discovery.h"
#include "../ble_lib_common.h"

// extern conn_peer_t m_connected_peers[NRF_SDH_BLE_TOTAL_LINK_COUNT];

// Random generated UUID
static const ble_uuid128_t imu_host_base_uuid = {
    .uuid128 = {0x1E, 0x94, 0x8D, 0xF1, 0x48, 0x31, 0x94, 0xBA, 0x75, 0x4C, 0x3E, 0x50, 0x02, 0x00, 0x3D, 0x71} // UART TX char UUID
};


static const ble_uuid128_t imu_base_uuid = { 
    .uuid128 = {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

uint32_t imu_uuids_init(ble_dev_c_t * p_ble_imu_c, ble_dev_c_t * p_ble_host_imu_c, ble_dev_c_init_t * p_ble_imu_c_init){

    uint32_t      err_code;
    ble_uuid_t    imu_uuid;

    VERIFY_PARAM_NOT_NULL(p_ble_imu_c);
    VERIFY_PARAM_NOT_NULL(p_ble_imu_c_init);

    // TO DO: add this line once we add a custom UUID
    // err_code = sd_ble_uuid_vs_add(&imu_base_uuid, &p_ble_imu_c->uuid_type);
    // APP_ERROR_CHECK(err_code);
    
    err_code = sd_ble_uuid_vs_add(&imu_base_uuid, &p_ble_imu_c->uuid_type);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_uuid_vs_add(&imu_host_base_uuid, &p_ble_host_imu_c->uuid_type);
    APP_ERROR_CHECK(err_code);
    // err_code = sd_ble_uuid_vs_add(&imu_host_base_uuid, &p_ble_host_imu_c->uuid_type);
    // APP_ERROR_CHECK(err_code);

    // for (uint32_t i = 0; i < TOTAL_LINK_COUNT; i++)
    // {
    //     m_connected_peers[i].state  = IDLE;
    // }

    imu_uuid.type = p_ble_imu_c->uuid_type;
    imu_uuid.uuid = IMU_SERVICE_UUID;

    p_ble_imu_c->conn_handle           = BLE_CONN_HANDLE_INVALID;
    p_ble_imu_c->evt_handler           = p_ble_imu_c_init->evt_handler;
    p_ble_imu_c->handles.dev_rx_handle = BLE_GATT_HANDLE_INVALID;
    p_ble_imu_c->handles.dev_tx_handle = BLE_GATT_HANDLE_INVALID;

    return ble_db_discovery_evt_register(&imu_uuid);
}

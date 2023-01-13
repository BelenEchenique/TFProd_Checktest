#include "sdk_macros.h"
#include "fes_uuids.h"
#include "app_error.h"
#include "ble.h"
#include "ble_db_discovery.h"
#include "../ble_lib_common.h"

static const ble_uuid128_t fes_host_base_uuid = {
    .uuid128 = {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00 , 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

uint32_t fes_uuids_init(ble_dev_c_t * p_ble_fes_c, ble_dev_c_init_t * p_ble_fes_c_init){

    uint32_t      err_code;
    ble_uuid_t    fes_uuid;

    VERIFY_PARAM_NOT_NULL(p_ble_fes_c);
    VERIFY_PARAM_NOT_NULL(p_ble_fes_c_init);

    // TO DO: add this line once we add a custom UUID
    // err_code = sd_ble_uuid_vs_add(&fes_base_uuid, &p_ble_fes_c->uuid_type);
    // APP_ERROR_CHECK(err_code);
    
    err_code = sd_ble_uuid_vs_add(&fes_host_base_uuid, &p_ble_fes_c->uuid_type);
    APP_ERROR_CHECK(err_code);

    fes_uuid.type = p_ble_fes_c->uuid_type;
    fes_uuid.uuid = FES_SERVICE_UUID;

    p_ble_fes_c->conn_handle           = BLE_CONN_HANDLE_INVALID;
    p_ble_fes_c->evt_handler           = p_ble_fes_c_init->evt_handler;
    p_ble_fes_c->handles.dev_rx_handle = BLE_GATT_HANDLE_INVALID;
    p_ble_fes_c->handles.dev_tx_handle = BLE_GATT_HANDLE_INVALID;

    return ble_db_discovery_evt_register(&fes_uuid);
}
#ifndef _FES_UUIDS_H
#define _FES_UUIDS_H

#include <stdint.h>
#include "fes_ble.h"
#include "../ble_lib_common.h"


#define FES_UUIDS_TRAINFES_BT_SIG_COMPANY_ID 0x86A8

#define FES_SERVICE_UUID            0x2220 // Heart rate service UUID
#define FES_SERVICE_UUID_TYPE     (g_fes_uuid_type)
#define FES_SVC_RX_CHAR_UUID        0x2221 // UART RX char UUID
#define FES_SVC_RX_CHAR_UUID_TYPE (g_fes_uuid_type)
#define FES_SVC_TX_CHAR_UUID        0x2222 // UART TX char UUID
#define FES_SVC_TX_CHAR_UUID_TYPE (g_fes_uuid_type)

uint32_t fes_uuids_init(ble_dev_c_t * p_ble_fes_c, ble_dev_c_init_t * p_ble_fes_c_init);

#endif // FES_UUIDS_H
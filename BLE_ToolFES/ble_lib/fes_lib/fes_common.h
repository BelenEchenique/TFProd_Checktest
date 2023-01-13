#ifndef _FES_COMMON_H_
#define _FES_COMMON_H_

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

#ifndef UUID16_SIZE
#define UUID16_SIZE             2                               /**< Size of 16 bit UUID */
#define UUID32_SIZE             4                               /**< Size of 32 bit UUID */
#define UUID128_SIZE            16                              /**< Size of 128 bit UUID */
#define BLE_FES_MAX_DATA_LEN    20
#define CLIENT_BUFFER_ALLOW     0x00
#define CLIENT_BUFFER_FULL      0x01
#define CLIENT_BUFFER_EXISTS    0x02
#endif

#define CLIENTS_FES_MAX_SIZE    10                              /**< CLIENTS buffer size. */
// typedef struct{
//     uint16_t            size;                 /**< Number of array entries. */
//     uint8_t             p_data[10];           /**< Pointer to array entries. */
// } uint8_fes_array_c_t;

// // typedef struct{
// //     uint8_t             buffer[6][CLIENTS_FES_MAX_SIZE];
// //     uint8_t             id[2][CLIENTS_FES_MAX_SIZE];
// //     uint8_t             model[CLIENTS_FES_MAX_SIZE];
// //     uint8_t             last_client;
// //     uint8_fes_array_c_t num_serial[CLIENTS_FES_MAX_SIZE];
// //     ble_gap_addr_t      peer_addr[CLIENTS_FES_MAX_SIZE];  
// // } clients_t;

// // typedef struct chosen_fes{
// //     ble_gap_addr_t      fes_addr;
// //     uint8_fes_array_c_t num_serial;
// //     uint8_t             id[2];
// // } chosen_t;

// /**@brief FES Client event type. */
// typedef enum{
//     BLE_FES_C_EVT_DISCOVERY_COMPLETE = 1, /**< Event indicating that the FES service and its characteristics was found. */
//     BLE_FES_C_EVT_FES_RX_EVT,             /**< Event indicating that the central has received something from a peer. */
//     BLE_FES_C_EVT_DISCONNECTED            /**< Event indicating that the FES server has disconnected. */
// } ble_fes_c_evt_type_t;

// /**@brief Handles on the connected peer device needed to interact with it.
// */
// typedef struct{
//     uint16_t                fes_rx_handle;      /**< Handle of the FES RX characteristic as provided by a discovery. */
//     uint16_t                fes_rx_cccd_handle; /**< Handle of the CCCD of the FES RX characteristic as provided by a discovery. */
//     uint16_t                fes_tx_handle;      /**< Handle of the FES TX characteristic as provided by a discovery. */
// } ble_fes_c_handles_t;

// /** @brief Structure containing the FES event data received from the peer. */
// typedef struct{
//     ble_fes_c_evt_type_t evt_type;
//     uint16_t             conn_handle;
//     uint8_t            * p_data;
//     uint8_t              data_len;
//     ble_fes_c_handles_t  handles;     /**< Handles on which the Nordic Uart service characteristics was discovered on the peer device. This will be filled if the evt_type is @ref BLE_FES_C_EVT_DISCOVERY_COMPLETE.*/
// } ble_fes_c_evt_t;


// // Forward declaration of the ble_fes_t type.
// typedef struct ble_fes_c_s ble_fes_c_t;

// /** @brief   Event handler type.
//  *
//  * @details This is the type of the event handler that should be provided by the application
//  *          of this module to receive events.
//  */
// typedef void (* ble_fes_c_evt_handler_t)(ble_fes_c_t * p_ble_fes_c, const ble_fes_c_evt_t * p_evt);


// /**@brief FES Client structure.
//  */
// struct ble_fes_c_s
// {
//     uint8_t                 uuid_type;          /**< UUID type. */
//     uint16_t                conn_handle;        /**< Handle of the current connection. Set with @ref ble_fes_c_handles_assign when connected. */
//     ble_fes_c_handles_t     handles;            /**< Handles on the connected peer device needed to interact with it. */
//     ble_fes_c_evt_handler_t evt_handler;        /**< Application event handler to be called when there is an event related to the FES. */
// };

// /**@brief FES Client initialization structure.
//  */
// typedef struct {
//     ble_fes_c_evt_handler_t evt_handler;
// } ble_fes_c_init_t;

#endif
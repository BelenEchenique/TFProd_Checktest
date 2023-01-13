#ifndef _BLE_LIB_COMMON_H_
#define _BLE_LIB_COMMON_H_

#include <stdbool.h>
#include "ble.h"
// #include "fes_lib/fes_uuids.h"
// #include "imu_lib/imu_uuids.h"
#include "app_util.h"
#include "ble_gap.h"
#include "nrf_error.h"
#include "bsp.h"
#include "ble_gap.h"
#include "ble_advdata.h"
#include "ble_db_discovery.h"
// #include "fes_lib/fes_common.h"
// #include "imu_lib/imu_common.h"

#ifndef UUID16_SIZE
#define UUID16_SIZE             2                               /**< Size of 16 bit UUID */
#define UUID32_SIZE             4                               /**< Size of 32 bit UUID */
#define UUID128_SIZE            16                              /**< Size of 128 bit UUID */
#define BLE_IMU_MAX_DATA_LEN    20
#define BLE_FES_MAX_DATA_LEN    20
#define CLIENT_BUFFER_ALLOW     0x00
#define CLIENT_BUFFER_FULL      0x01
#define CLIENT_BUFFER_EXISTS    0x02
#endif

#define ERROR_CLIENT_NUM        20
#define SENSOR_PACKAGES         20
#define PWM_PIN                 19

#define CLIENTS_MAX_SIZE        10                              /**< CLIENTS buffer size. */
#define CLIENTS_FES_MAX_SIZE    10                              /**< CLIENTS buffer size. */

#define TRIGGER_CONNECTION      '!'
#define MASTER_PASSWORD "76312045"

#define CENTRAL_LINK_COUNT      1                                           /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT   0                                           /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#define TOTAL_LINK_COUNT        CENTRAL_LINK_COUNT + PERIPHERAL_LINK_COUNT  /**< Total number of links used by the application. */

/* Sensor*/
#define SENSOR_PACKAGES         20

/* States of connection */
typedef enum state_connection_fes{
    STAND_BY,
    SCAN_ON,
    CONNECT_IMU,
    CONNECT_FES
} state_connection_dev_t;

/* State of Buffer */
typedef enum state_buffer{
    NOREADY,
    READY
} state_buffer_t;

typedef enum stream_mode{
    OFF,
    ON
} stream_mode_t;

typedef struct sensor_data{
    uint8_t   index_data_type;
    uint32_t  data_sensor[SENSOR_PACKAGES];
    long double  data_decoded[10];
} sensor_data_t;

/* States of Testing */
typedef enum state_test{NONE, ERROR, INIT_EFFORT,
    TRIGGERCONNECTION      = TRIGGER_CONNECTION,   // ! in the first element of the array
    CHANGE_DEVICE_TO_IMU   = '/',                   // Trigger for exit tests
    CHANGE_DEVICE_TO_FES   = '-',                   // Trigger for exit tests
    DISCONNECT             = '^',

    // Clients
    CLIENT1                = '1',  
    CLIENT2                = '2',   
    CLIENT3                = '3',   
    CLIENT4                = '4',  
    CLIENT5                = '5',  
    CLIENT6                = '6',  
    CLIENT7                = '7',  
    CLIENT8                = '8',  
    CLIENT9                = '9',  
    CLIENT10               = 'A',  

    // Electric test
    HVTEST                 = '}',                  // Trigger for high voltage test
    SERIALNUM              = '&',                  // Trigger for returning serial number
    EXITTEST               = '?',                   // Trigger for exit tests

    OTAUPDATE              = 'o',                  // Trigger for high voltage test
    BATTERY                = '(',                  // Trigger for adquisition of morfology wave
    CLR                    = '~',                  // Trigger for returning serial number
    KINETICS               = '<',                  // Trigger for PWM angles test
    END_KINETICS           = '>',                  // Trigger for PWM angles test
    AUTONOMY               = '%',
    STARTSTREAM            = '#',                  // Trigger for exit tests
    ENDSTREAM              = '$',                  // Trigger for exit tests
    RESET                  = '_',                  // Trigger for exit tests
    INFO                   = '*',                  // Trigger for exit tests
    SEND_STREAMING         = 'S'
} state_test_t;

typedef struct{
    uint16_t            size;                 /**< Number of array entries. */
    uint8_t             p_data[10];           /**< Pointer to array entries. */
} uint8_array_c_t;

typedef struct{
    uint8_t             buffer[6][CLIENTS_MAX_SIZE];
    uint8_t             id[2][CLIENTS_MAX_SIZE];
    uint8_t             model[CLIENTS_MAX_SIZE];
    uint8_t             last_client;
    uint8_array_c_t     num_serial[CLIENTS_MAX_SIZE];
    ble_gap_addr_t      peer_addr[CLIENTS_MAX_SIZE];  
} clients_t;

typedef struct chosen_dev{
    ble_gap_addr_t      dev_addr;
    uint8_array_c_t     num_serial;
    uint8_t             id[2];
} chosen_t;

/**@brief FES Client event type. */
typedef enum{
    BLE_DEV_C_EVT_DISCOVERY_COMPLETE = 1, /**< Event indicating that the FES service and its characteristics was found. */
    BLE_DEV_C_EVT_DEV_RX_EVT,             /**< Event indicating that the central has received something from a peer. */
    BLE_DEV_C_EVT_DISCONNECTED            /**< Event indicating that the FES server has disconnected. */
} ble_dev_c_evt_type_t;

/**@brief Handles on the connected peer device needed to interact with it.
*/
typedef struct{
    uint16_t                dev_rx_handle;      /**< Handle of the FES RX characteristic as provided by a discovery. */
    uint16_t                dev_rx_cccd_handle; /**< Handle of the CCCD of the FES RX characteristic as provided by a discovery. */
    uint16_t                dev_tx_handle;      /**< Handle of the FES TX characteristic as provided by a discovery. */
} ble_dev_c_handles_t;

/** @brief Structure containing the FES event data received from the peer. */
typedef struct{
    ble_dev_c_evt_type_t evt_type;
    uint16_t             conn_handle;
    uint8_t            * p_data;
    uint8_t              data_len;
    ble_dev_c_handles_t  handles;     /**< Handles on which the Nordic Uart service characteristics was discovered on the peer device. This will be filled if the evt_type is @ref BLE_FES_C_EVT_DISCOVERY_COMPLETE.*/
} ble_dev_c_evt_t;


// Forward declaration of the ble_fes_t type.
typedef struct ble_dev_c_s ble_dev_c_t;

/** @brief   Event handler type.
 *
 * @details This is the type of the event handler that should be provided by the application
 *          of this module to receive events.
 */
typedef void (* ble_dev_c_evt_handler_t)(ble_dev_c_t * p_ble_dev_c, const ble_dev_c_evt_t * p_evt);


/**@brief FES Client structure.
 */
struct ble_dev_c_s
{
    uint8_t                 uuid_type;          /**< UUID type. */
    uint16_t                conn_handle;        /**< Handle of the current connection. Set with @ref ble_dev_c_handles_assign when connected. */
    ble_dev_c_handles_t     handles;            /**< Handles on the connected peer device needed to interact with it. */
    ble_dev_c_evt_handler_t evt_handler;        /**< Application event handler to be called when there is an event related to the FES. */
};

/**@brief FES Client initialization structure.
 */
typedef struct {
    ble_dev_c_evt_handler_t evt_handler;
} ble_dev_c_init_t;

/**@brief Client states. */
typedef enum
{
    IDLE,                                           /**< Idle state. */
    STATE_SERVICE_DISC,                             /**< Service discovery state. */
    STATE_NOTIF_ENABLE,                             /**< State where the request to enable notifications is sent to the peer. . */
    STATE_RUNNING,                                  /**< Running state. */
    STATE_ERROR                                     /**< Error state. */
} client_state_t;


typedef struct
{
        bool                   is_connected;
        char                   name[10];
        uint8_t                state;
        uint16_t               conn_handle; 
        ble_gap_addr_t         address;
        ble_dev_c_handles_t    handles;            /**< Handles on the connected peer device needed to interact with it. */
} conn_peer_t;

#endif
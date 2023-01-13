#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nordic_common.h"
#include "app_error.h"
#include "app_uart.h"
#include "ble_db_discovery.h"
#include "app_timer.h"
#include "app_util.h"
#include "bsp.h"
#include "boards.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_hci.h"
#include "softdevice_handler.h"
#include "ble_advdata.h"

// MB libraries
#include "ble_lib/ble_connection.h"
#include "ble_lib/ble_lib_common.h"
#include "ble_lib/fes_lib/fes_uuids.h"
#include "ble_lib/fes_lib/fes_ble.h"
#include "ble_lib/fes_lib/fes_tests.h"
#include "ble_lib/imu_lib/imu_uuids.h"
#include "ble_lib/imu_lib/imu_ble.h"
#include "ble_lib/imu_lib/imu_tests.h"
#include "ble_lib/ble_fsm.h"

#if (NRF_SD_BLE_API_VERSION == 3)
#define NRF_BLE_MAX_MTU_SIZE    GATT_MTU_SIZE_DEFAULT           /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */
#endif

#define UART_TX_BUF_SIZE        256                             /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        256                             /**< UART RX buffer size. */

#define NUS_SERVICE_UUID_TYPE   BLE_UUID_TYPE_VENDOR_BEGIN      /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_TIMER_PRESCALER     0                               /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 2                               /**< Size of timer operation queues. */

#define SCAN_INTERVAL           0x00A0                          /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW             0x0050                          /**< Determines scan window in units of 0.625 millisecond. */
#define SCAN_ACTIVE             1                               /**< If 1, performe active scanning (scan requests). */
#define SCAN_SELECTIVE          0                               /**< If 1, ignore unknown devices (non whitelisted). */
#define SCAN_TIMEOUT            0x0000                          /**< Timout when scanning. 0x0000 disables timeout. */

#define MIN_CONNECTION_INTERVAL MSEC_TO_UNITS(20, UNIT_1_25_MS) /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL MSEC_TO_UNITS(75, UNIT_1_25_MS) /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY           0                               /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT     MSEC_TO_UNITS(3000, UNIT_10_MS) /**< Determines supervision time-out in units of 10 millisecond. */

ble_dev_c_t                     m_ble_dev_c;                    /**< Instance of NUS service. Must be passed to all NUS_C API calls. */
ble_dev_c_t                     m_ble_host_imu_c;               /**< Instance of NUS service. Must be passed to all NUS_C API calls. */
static ble_db_discovery_t       m_ble_db_discovery;             /**< Instance of database discovery module. Must be passed to all db_discovert API calls */

static chosen_t                 fes_chosen;                     /**< struct of fes chosen by production user. */
static chosen_t                 imu_chosen;                     /**< struct of fes chosen by production user. */
clients_t                       Clients_dev;                        /**< Instance of fes clients in buffer struct. */

volatile state_connection_dev_t state_dev   = STAND_BY;         /**< state of connection on BLE */
volatile state_test_t           state_test  = NONE;             /**< state of testing in FES. */ 
volatile bool                   fes_adv     = true;             /**< either FES or WALK */
volatile uint8_t                mins = 240;                     /**< minutes for effort test */

volatile uint8_t                buffer[BLE_FES_MAX_DATA_LEN];   /**< RX Buffer UART . */
volatile uint8_t                indx = 0;                      /**< Index of RX Fuffer. */
volatile state_buffer_t         rx_buf   = NOREADY;             /**< State of RX Buffer. */

// IMU-BLE handler
uint8_t           hdlr       = 0;
stream_mode_t     streaming; 
uint8_t           kinetic_test  = 0; 
uint8_t           autonomy_test = 0; 
uint8_t           servo; 
stream_mode_t     flag_send_streaming; 
// conn_peer_t       m_connected_peers[TOTAL_LINK_COUNT];
// uint8_t           client_counter; 
// uint8_t           aux_client_counter; 
// sensor_data_t     data_sensor_a;
// uint8_t           buffer_counter[TOTAL_LINK_COUNT]; 

/**
 * @brief Connection parameters requested for connection.
 */
static const ble_gap_conn_params_t m_connection_param ={
    (uint16_t)MIN_CONNECTION_INTERVAL,  // Minimum connection
    (uint16_t)MAX_CONNECTION_INTERVAL,  // Maximum connection
    (uint16_t)SLAVE_LATENCY,            // Slave latency
    (uint16_t)SUPERVISION_TIMEOUT       // Supervision time-out
  };

/**
 * @brief Parameters used when scanning.
 */
static const ble_gap_scan_params_t m_scan_params ={
    .active   = 1,
    .interval = SCAN_INTERVAL,
    .window   = SCAN_WINDOW,
    .timeout  = SCAN_TIMEOUT,
    #if (NRF_SD_BLE_API_VERSION == 2)
        .selective   = 0,
        .p_whitelist = NULL,
    #endif
    #if (NRF_SD_BLE_API_VERSION == 3)
        .use_whitelist = 0,
    #endif
};

/** @brief Function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing ASSERT call.
 * @param[in] p_file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
};

/** @brief Function to start scanning.
 */
static void scan_start(void)
{
    static bool scan_already = false;
    ret_code_t ret;
    emptyClients(&Clients_dev);
    if (state_dev == CONNECT_FES){
        sd_ble_gap_disconnect(m_ble_dev_c.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);   
        scan_already = false;
    }
    else if (state_dev == CONNECT_IMU){
        sd_ble_gap_disconnect(m_ble_dev_c.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);   
        sd_ble_gap_disconnect(m_ble_host_imu_c.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);   
        scan_already = false;
    }


    if (scan_already == false){
        ret = sd_ble_gap_scan_start(&m_scan_params);
        APP_ERROR_CHECK(ret);
        scan_already = true;
    }
    state_dev = SCAN_ON;
}


/** @brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_fes_c_on_db_disc_evt(&m_ble_dev_c, p_evt);
    ble_imu_c_on_db_disc_evt(&m_ble_dev_c, p_evt);
}

/** @brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' i.e '\r\n' (hex 0x0D) or if the string has reached a length of
 *          @ref NUS_MAX_DATA_LENGTH.
 */
void uart_event_handle(app_uart_evt_t * p_event)
{
    switch (p_event->evt_type)  
    {
        /**@snippet [Handling data from UART] */
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get((uint8_t *) (buffer + indx)));
            if (buffer[indx] == '\n'){
                rx_buf = READY;
            }
            indx++;
            break;
        /**@snippet [Handling data from UART] */
        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;
        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;
        default:
            break;
    }
}

/** @brief Callback handling FES Client events.
 *
 * @details This function is called to notify the application of FES client events.
 *
 * @param[in]   p_ble_fes_c   FES Client Handle. This identifies the FES client
 * @param[in]   p_ble_fes_evt Pointer to the FES Client event.
 */
/**@snippet [Handling events from the ble_fes_c module] */
static void ble_fes_c_evt_handler(ble_dev_c_t * p_ble_fes_c, const ble_dev_c_evt_t * p_ble_fes_evt)
{
    uint32_t err_code;
    switch (p_ble_fes_evt->evt_type)
    {
        case BLE_DEV_C_EVT_DISCOVERY_COMPLETE:
            err_code = ble_fes_c_handles_assign(p_ble_fes_c, p_ble_fes_evt->conn_handle, &p_ble_fes_evt->handles);
            APP_ERROR_CHECK(err_code);

            err_code = ble_fes_c_rx_notif_enable(p_ble_fes_c);
            APP_ERROR_CHECK(err_code);

            COMMAND_FES_BLE(MASTER_PASSWORD);
            break;

        case BLE_DEV_C_EVT_DEV_RX_EVT:
            for (uint32_t i = 0; i < p_ble_fes_evt->data_len; i++)
            {
                while (app_uart_put(p_ble_fes_evt->p_data[i]) != NRF_SUCCESS); 
            }
            while (app_uart_put('\r') != NRF_SUCCESS);
            while (app_uart_put('\n') != NRF_SUCCESS);
            break;

        case BLE_DEV_C_EVT_DISCONNECTED:
            scan_start();
            // break;
    }
}

/** @brief Callback handling IMU Client events.
 *
 * @details This function is called to notify the application of IMU client events.
 *
 * @param[in]   p_ble_imu_c   IMU Client Handle. This identifies the IMU client
 * @param[in]   p_ble_imu_evt Pointer to the IMU Client event.
 */
/**@snippet [Handling events from the ble_imu_c module] */
// static void ble_imu_c_evt_handler(ble_dev_c_t * p_ble_imu_c, const ble_dev_c_evt_t * p_ble_imu_evt, conn_peer_t m_connected_peers[hdlr])
static void ble_imu_c_evt_handler(ble_dev_c_t * p_ble_imu_c, const ble_dev_c_evt_t * p_ble_imu_evt)
{
    uint32_t err_code;

    switch (p_ble_imu_evt->evt_type)
    {
        case BLE_DEV_C_EVT_DISCOVERY_COMPLETE:
            err_code = ble_imu_c_handles_assign(p_ble_imu_c, p_ble_imu_evt->conn_handle, &p_ble_imu_evt->handles);
            APP_ERROR_CHECK(err_code);

            err_code = ble_imu_c_rx_notif_enable(p_ble_imu_c);
            APP_ERROR_CHECK(err_code);

            break;

        case BLE_DEV_C_EVT_DEV_RX_EVT:
            if (p_ble_imu_evt->p_data[0] == 0x80 || p_ble_imu_evt->p_data[0] == 0x2A) {
                int init; 
                char buf[30]; 
                init = 0; 
                switch (p_ble_imu_evt->p_data[1]){
                    case 0x16:
                        sprintf(buf, "batt;%d,%d\r\n", p_ble_imu_evt->p_data[4], p_ble_imu_evt->p_data[5]);
                        PRINT_TO_ATSAM(buf);
                        break;
                    default: 
                        if (flag_send_streaming == ON){
                            flag_send_streaming = OFF; 
                            for (uint32_t i = init ; i < p_ble_imu_evt->data_len; i++){
                                while (app_uart_put(p_ble_imu_evt->p_data[i]) != NRF_SUCCESS);
                            }
                            while (app_uart_put('\r') != NRF_SUCCESS);
                            while (app_uart_put('\n') != NRF_SUCCESS);
                        }
                        break;
                    }
                }
            //     else{
                    // for (uint32_t i = 0 ; i < p_ble_imu_evt->data_len; i++)
                    //     {
                    //         while (app_uart_put(p_ble_imu_evt->p_data[i]) != NRF_SUCCESS);
                    //         // else {
                    //         //         decodeSensorStreamedData (p_ble_imu_evt->p_data[i], &data_sensor_a, buffer_counter[0]);
                    //         //         // buffer_counter[0] = buffer_counter[0] == 3 ? 0 : buffer_counter[0] + 1 ; 
                    //         // }
                    //     }                
                    //     // }
            break;

        case BLE_DEV_C_EVT_DISCONNECTED:
            PRINT_TO_ATSAM("IMU disconnected");
            scan_start();
            // state_dev = SCAN_ON;
            break;
        default: 
            break;
    }
}

/** @brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt){
    uint32_t err_code;
    // uint16_t conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    const ble_gap_evt_t * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_ADV_REPORT:
            if (selected_device == FES_DEVICE_SELECTED){
                on_adv_report_fes(p_ble_evt, &Clients_dev, p_gap_evt);
            }
            else if (selected_device == IMU_DEVICE_SELECTED){
                on_adv_report_imu(p_ble_evt, &Clients_dev, p_gap_evt);
            }
            break; // BLE_GAP_EVT_ADV_REPORT

        case BLE_GAP_EVT_CONNECTED:
            PRINT_TO_ATSAM("=");
            if (selected_device == IMU_DEVICE_SELECTED){
                write_chosen_imu(&imu_chosen);
                PRINT_TO_ATSAM("\r\n");
                state_dev = CONNECT_IMU;  
            }

            else if (selected_device == FES_DEVICE_SELECTED){
                for(uint16_t i = 0 ; i < fes_chosen.num_serial.size ; i++)
                    CHAR_TO_ATSAM(fes_chosen.num_serial.p_data[i]);
                PRINT_TO_ATSAM("\r\n");
                state_dev = CONNECT_FES;  
            }

            // start discovery of services. The FES Client waits for a discovery result
            err_code = ble_db_discovery_start(&m_ble_db_discovery, p_ble_evt->evt.gap_evt.conn_handle);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GAP_EVT_TIMEOUT:
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
            {
                PRINT_TO_ATSAM("Scan timeout.\r\n");
                scan_start();
            }
            else if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {

                PRINT_TO_ATSAM("Connection timed out.\r\n");
                scan_start();
            }
            break; // BLE_GAP_EVT_TIMEOUT

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
           break; // BLE_GAP_EVT_SEC_PARAMS_REQUEST
   APP_ERROR_CHECK(err_code);
          
        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            // Accepting parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            //NRF_LOG_DEBUG("GATT Client Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            PRINT_TO_ATSAM("*DisConnect\n"); 
            scan_start();
            break; // BLE_GATTC_EVT_TIMEOUT

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            //NRF_LOG_DEBUG("GATT Server Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            PRINT_TO_ATSAM("*DisConnect\n"); 
            scan_start();
            break; // BLE_GATTS_EVT_TIMEOUT
        case BLE_GAP_EVT_DISCONNECTED:
            scan_start();
            PRINT_TO_ATSAM("*DisConnect\n"); 
            break;

#if (NRF_SD_BLE_API_VERSION == 3)
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                       NRF_BLE_MAX_MTU_SIZE);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
#endif

        default:
            break;
    }
}

/** @brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack event has
 *          been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    on_ble_evt(p_ble_evt);
    ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
    ble_fes_c_on_ble_evt(&m_ble_dev_c, p_ble_evt);
    // ble_imu_c_on_ble_evt(&m_ble_dev_c, p_ble_evt);
}

/** @brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void){
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
#if (NRF_SD_BLE_API_VERSION == 3)
    ble_enable_params.gatt_enable_params.att_mtu = NRF_BLE_MAX_MTU_SIZE;
#endif
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

/** @brief Function for initializing the UART. */
static void uart_init(void)
{
    uint32_t err_code;

    const app_uart_comm_params_t comm_params =
      {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
      };

    APP_UART_FIFO_INIT(&comm_params,
                        UART_RX_BUF_SIZE,
                        UART_TX_BUF_SIZE,
                        uart_event_handle,
                        APP_IRQ_PRIORITY_LOWEST,
                        err_code);

    APP_ERROR_CHECK(err_code);
}

/** @brief Function for initializing the FES Client.*/
static void fes_ble_init()
{
    uint32_t         err_code;
    ble_dev_c_init_t fes_c_init_t;

    fes_c_init_t.evt_handler = ble_fes_c_evt_handler;

    err_code = fes_uuids_init(&m_ble_dev_c, &fes_c_init_t);
    APP_ERROR_CHECK(err_code);
}

/** @brief Function for initializing the IMU Client.*/
static void imu_ble_init()
{
    uint32_t         err_code;
    ble_dev_c_init_t imu_c_init_t;

    imu_c_init_t.evt_handler = ble_imu_c_evt_handler;

    streaming         =  OFF; 

    err_code = imu_uuids_init(&m_ble_dev_c, &m_ble_host_imu_c, &imu_c_init_t);
    APP_ERROR_CHECK(err_code);
}

/** @brief Function for initializing the FES Client.*/
static void dev_ble_init(device_t selected_device)
{
    if (state_dev == CONNECT_FES){
        sd_ble_gap_disconnect(m_ble_dev_c.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);   
    }
    else if (state_dev == CONNECT_IMU){
        sd_ble_gap_disconnect(m_ble_dev_c.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);   
        sd_ble_gap_disconnect(m_ble_host_imu_c.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);   
    }

    switch (selected_device)
    {
    case FES_DEVICE_SELECTED:
        fes_ble_init(); 
        break;
    case IMU_DEVICE_SELECTED:
        imu_ble_init(); 
    default:
        break;
    }
}

/** @brief Function for initializing the Database Discovery Module. */
static void db_discovery_init(void)
{
    uint32_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}

/** @brief Function for the Power manager. */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/** @brief Function for handle test request UART*/
static void test_process(void)
{
    switch (state_test){
    case SEND_STREAMING: 
        flag_send_streaming = ON; 
        state_test = NONE; 
        break; 
    case HVTEST: // FES
        highVoltageTest(&state_test, &fes_adv);
        break;
    case SERIALNUM: // FES
        serialNumber(&state_test, &fes_adv);
        break;
    case EXITTEST: 
        COMMAND_FES_BLE("_F;0");
        state_test = NONE;
        break;
    case OTAUPDATE: // IMU
        otaUpdate(&state_test);
        PRINT_TO_ATSAM("OTAUPDATE\r\n");
        break;
    case BATTERY: // IMU
        batteryTest(&state_test, 1);
        break;
    case CLR: // IMU
        colorTest(&state_test);
        PRINT_TO_ATSAM("CLR\r\n");
        break; 
    case RESET: // IMU
        resetTest(&state_test);
        PRINT_TO_ATSAM("*DisConnect\n"); 
        break; 
    case INFO: // IMU
        infoTest(&state_test);
        PRINT_TO_ATSAM("INFO\r\n");
        break; 
    case STARTSTREAM: // IMU
        startStreamMode(&state_test);
        flag_send_streaming = ON; 
        break; 
    case ENDSTREAM: // IMU
        endStreamMode(&state_test);
        break; 
    case KINETICS:
        if (autonomy_test == 0 && kinetic_test == 0){
            batteryTest(&state_test, 10);
            streaming = ON; 
            flag_send_streaming = ON; 
            startStreamMode(&state_test);
            nrf_delay_ms(100);
            kinetic_test = 1; 
            // state_test = END_KINETICS; 
        }
        break; 
    case END_KINETICS: 
        PRINT_TO_ATSAM("END"); 
        kinetic_test = 0; 
        endStreamMode(&state_test);
        batteryTest(&state_test, 10);
        streaming = OFF; 
        break; 
    case AUTONOMY: // IMU
        if (kinetic_test == 0){
            switch (streaming){
                case OFF:
                    streaming = ON; 
                    flag_send_streaming = ON; 
                    startStreamMode(&state_test);
                    nrf_delay_ms(10);
                    autonomy_test = 1;
                    break;
                default:
                    streaming = OFF;
                    nrf_delay_ms(10);
                    endStreamMode(&state_test);
                    autonomy_test = 0; 
                    break;
            }
        }
        break;
    case NONE:
        PRINT_TO_ATSAM("NONE\r\n");
        break;
    default:
        break;
    }
}

/** @brief Function for handle connection request UART */
static void connection_process(void)
{
    if(buffer[1] == '\n' || state_dev == STAND_BY)
    {
        scan_start();
        state_dev = SCAN_ON;
    }
    else if (state_dev == SCAN_ON)
    {
        uint16_t int_client = own_atoi((uint8_t *) buffer);
        if (int_client > CLIENTS_MAX_SIZE || int_client == 0)
        {
            PRINT_TO_ATSAM("Not allowed, bad input\r\n");
        }
        else if (int_client > Clients_dev.last_client || Clients_dev.last_client == 0)
        {
            PRINT_TO_ATSAM("Not allowed, no client\r\n");
        }
        else
        {
            if (selected_device == FES_DEVICE_SELECTED){
                fes_chosen.dev_addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE;
                for(uint8_t i = 0 ; i < 6 ; i++)
                {
                    fes_chosen.dev_addr.addr[i] = Clients_dev.buffer[i][int_client-1];
                }
                fes_chosen.num_serial.size = Clients_dev.num_serial[int_client-1].size;
                for(uint8_t i = 0 ; i < fes_chosen.num_serial.size ; i++)
                {
                    fes_chosen.num_serial.p_data[i] = Clients_dev.num_serial[int_client-1].p_data[i];
                }
                fes_chosen.id[0] = Clients_dev.id[0][int_client-1];
                fes_chosen.id[1] = Clients_dev.id[1][int_client-1];
                sd_ble_gap_connect(&fes_chosen.dev_addr, &m_scan_params, &m_connection_param);
            }
            else if (selected_device == IMU_DEVICE_SELECTED){
                imu_chosen.dev_addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE;
                for(uint8_t i = 0 ; i < 6 ; i++)
                {
                    imu_chosen.dev_addr.addr[i] = Clients_dev.buffer[i][int_client-1];
                }
                imu_chosen.num_serial.size = Clients_dev.num_serial[int_client-1].size;
                for(uint8_t i = 0 ; i < imu_chosen.num_serial.size ; i++)
                {
                    imu_chosen.num_serial.p_data[i] = Clients_dev.num_serial[int_client-1].p_data[i];
                }
                imu_chosen.id[0] = Clients_dev.id[0][int_client-1];
                imu_chosen.id[1] = Clients_dev.id[1][int_client-1];
                sd_ble_gap_connect(&imu_chosen.dev_addr, &m_scan_params, &m_connection_param);
            }
        }
    }
    else if (state_dev == CONNECT_FES)
    {
        if ((buffer[indx - 1] == '\n') || (indx >= (BLE_FES_MAX_DATA_LEN)))
        {
            while (ble_fes_c_string_send(&m_ble_dev_c, (uint8_t *) (buffer + 1), indx - 2) != NRF_SUCCESS);
        }
    }
}

int main(void)
{
    process_type_t process = NOTHING;
    // selected_device = IMU_DEVICE_SELECTED; 
    selected_device = FES_DEVICE_SELECTED; 

    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);
    uart_init();
    db_discovery_init();
    ble_stack_init();
    // imu_ble_init();
    fes_ble_init();

    PRINT_TO_ATSAM("Reset\r\n");

    for (;;){
        // uint32_t err_code;
        power_manage();
        /* Toggle LEDs. */

        if (rx_buf == READY){
            process = buffer_process(buffer, &state_dev, &state_test, selected_device);
            rx_buf = NOREADY;
        }
        switch (process){
            case TEST:
                test_process();
                process = NOTHING;
                indx = 0;
                break;
            case CONNECTION:
                connection_process();
                process = NOTHING;
                indx = 0;
                break;
            case CHANGE_SETTINGS_TO_IMU:
                selected_device = IMU_DEVICE_SELECTED;
                dev_ble_init(selected_device); 
                process = NOTHING;
                indx = 0;
                break;
            case CHANGE_SETTINGS_TO_FES:
                selected_device = FES_DEVICE_SELECTED;
                dev_ble_init(selected_device); 
                process = NOTHING;
                indx = 0;
                break;
            case DISCONNECT_DEVICE:
                sd_ble_gap_disconnect(m_ble_dev_c.conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
                process = NOTHING;
                indx = 0;
                break;
            case ERROR_T:
                PRINT_TO_ATSAM("Error \r\n");
                process = NOTHING;
                indx = 0;
                break;
            case UNKNOWN_COMMAND:
                PRINT_TO_ATSAM("Unknown \r\n");
                process = NOTHING;
                indx = 0;
                break;
            default:
                break;
        }
    }
}

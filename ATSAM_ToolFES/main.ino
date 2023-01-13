#include <Servo.h>
#include <FlashStorage_SAMD.h>
#include <SPI.h>
#include <SD.h>
#include "fix16.h"

/* States */
typedef enum state{STAND_BY, TP2,TP4,TP5, TP6,TP6hv,TP9,TP10,TP10hv, TPALL,
                    STIMULATE,SERIALNUM,TP, CALIBRATE, CALIBRATE_HV, CALIB_VALS, RELAY1_ON, 
                    RELAY2_ON, RELAY_OFF,  
                    CONNECTING_BLE, BLE_READY, BLE_COMMAND_FULL, ERROR, RESET,
                    SET_R1 = 'A', SET_R2 = 'B', SET_R3 = 'C', SET_R4 = 'D',  SET_R5 = 'E',  SET_R6 = 'F', 
                    SET_R7 = 'G', SET_R8 = 'H', SET_R9 = 'I', SET_R10 = 'J', SET_R11 = 'K', SET_R12 = 'L',
                    KINETIC_TEST, AUTONOMY_TEST, END_AUTONOMY
} state_t;

typedef enum set_ranges{
    NOT_ASSIGNED, 
    SET_LOWCUT  = 'l', 
    SET_HIGHCUT = 'h', 
    SET_AVERAGE = 'a', 
} set_ranges_t;

typedef enum setting_mode{
    OFF,
    RESISTOR_SETTING, 
    TP_SETTING, 
    VREF1_SETTING,
    VREF2_SETTING,
    VREFhv_SETTING
} setting_mode_t;

typedef struct tp_ranges_config{
    float min_value; 
    float max_value; 
    float average_value;     
} tp_ranges;

typedef struct voltage_divs{
    double VDIV_TP2; 
    double VDIV_TP4; 
    double VDIV_TP5; 
    double VDIV_TP6; 
    double VDIV_TP9;     
    double VDIV_TP10;     
    double VDIV_TP6hv; 
} voltage_divs_t;

typedef struct V0{
    double V0_TP2; 
    double V0_TP4; 
    double V0_TP5; 
    double V0_TP6; 
    double V0_TP9;     
    double V0_TP10;     
    double V0_TP6hv; 
} offset_voltage_t;

typedef struct vx{
    double vx1; 
    double vx2; 
} vx_t;

typedef struct v_autocalib{
    vx_t Vx_TP2; 
    vx_t Vx_TP4; 
    vx_t Vx_TP5; 
    vx_t Vx_TP6; 
    vx_t Vx_TP9;     
    vx_t Vx_TP10;    
    vx_t Vx_TP6hv;    
} v_autocalib_t;

typedef enum state_servo{
    NONE,
    DIR1,   
    DIR2,
    DIR3, 
    DIR4,
    DIR5,
    DIR6,   
    DIR7,
    DIR8, 
    DIR9,
    DIR10,
    DIR11
} state_servo_t;



typedef enum orientation{
    FIRST  = 0,
    SECOND = 1,
    THIRD  = 2,
    FOURTH = 3,
    FIFTH  = 4,
    CLKWISE,
    ANTICLKWISE,
    BOTH
} orientation_t; 


typedef struct calibrated_values{
    offset_voltage_t V0;
    voltage_divs_t   voltage_divs; 
} calibrated_values_t;

typedef struct VREFS{
    float Vref1; 
    float Vref2; // mV
    float Vrefhv; // mV
}vref_t; 

typedef struct servos{
    Servo servo1;
    Servo servo2; 
} servos_t;

typedef enum stream_mode{
    OFF_stream,
    PREPARE_stream,
    ON_stream
} stream_mode_t;

/* Tolerancia para mediciones en test points */
typedef struct tpconfig{
    uint8_t TP1tolerance; //              // Porcentaje de tolerancia de TP1 
    uint8_t TP2tolerance; //              // Porcentaje de tolerancia de TP2
    uint8_t TP3tolerance; //  0 g          // Porcentaje de tolerancia de TP3 
    uint8_t TP4tolerance; //  3           // Porcentaje de tolerancia de TP4
    uint8_t TP5tolerance; //  3           // Porcentaje de tolerancia de TP5
    uint8_t TP6tolerance; //  5           // Porcentaje de tolerancia de TP6
    uint8_t TP7tolerance; //  6           // Porcentaje de tolerancia de TP7
    uint8_t TP8tolerance; //              // Porcentaje de tolerancia de TP8
    uint8_t TP9tolerance; //  5           // Porcentaje de tolerancia de TP9
    uint8_t TP10tolerance; // 5           // Porcentaje de tolerancia de TP10
} tol_config;

typedef enum electro_num
{
    CH_tp2  = A0,
    CH_tp9  = A1,
    CH_tp4  = A2,    
    CH_tp5  = A3,
    CH_tp10 = A4, 
    CH_tp6  = A5,
    CH_GND_NUM,
    I_CHARGE_NUM
} nums;

/* TP SETTINGS */
#define MINIMUM 1
#define MAXIMUM 2
#define AVERAGE 3

/* UART*/
#define FES                1U
#define WALKFES            2U
#define USB_RXBUFFER_SIZE  40U
#define BLE_RXBUFFER_SIZE  40U
#define SETTING_MODE       'Z'
#define END_SETTINGS_MODE  'Y'
#define INPUT_FES          'X'
#define INPUT_WFES         'W'
#define TESTPOINT          'T'
#define CHANGE_DEV_TO_IMU  '~'
#define AUTOCALIBRATION    'V'
#define AUTOCALIBRATION_HV 'U'
// #define READ_CALIB_VALS    'U'
#define TEST_REF1          'R'
#define TEST_REF2          'Q'
#define SET_REF1           'm'
#define SET_REF2           'n'
#define SET_REFhv          'o'
#define TEST_tps           'r'
#define CHANGE_DEV_TO_FES  '-'
#define YES                 1
#define NO                  0
#define START_KINETIC_TEST  'k'
#define START_AUTONOMY_TEST 'a'
#define END_AUTONOMY_TEST   'e'

/* Resistencias */
#define R1_default  237000 
#define R2_default  0 
#define R3_default  237000 
#define R4_default  237000 
#define R5_default  237000 
#define R6_default  237000 
#define R7_default  237000
#define R8_default  237000 
#define R9_default  10000 
#define R10_default 237000 
#define R11_default 3920 
#define R12_default 3920 

/* Resistencias */
volatile uint32_t R1  = R1_default; 
volatile uint32_t R2  = R2_default;
volatile uint32_t R3  = R3_default; 
volatile uint32_t R4  = R4_default; 
volatile uint32_t R5  = R5_default; 
volatile uint32_t R6  = R6_default; 
volatile uint32_t R7  = R7_default; 
volatile uint32_t R8  = R8_default; 
volatile uint32_t R9  = R9_default;
volatile uint32_t R10 = R10_default; 
volatile uint32_t R11 = R11_default;
volatile uint32_t R12 = R12_default;

// Create variables
volatile          boolean trigger_a_int = false;
volatile state_t  state = STAND_BY;
volatile int      tolerance = 0; 
volatile int      nresistor = -1; 
int               target;
int result_bat, result_sbat, result_3v3_TP4, result_3v3_TP5, result_HV, result_pump, result_stim; 

// UART
volatile uint8_t connected_to_FES = NO;
volatile uint8_t connected_to_IMU = NO;
volatile setting_mode_t setting_mode = OFF;
String serialUSB_RingBuffer = "";         // a String to hold incoming data
String serial1_RingBuffer = "";         // a String to hold incoming data
uint8_t serial1_RingBuffer_int[30] ;         // a String to hold incoming data
uint8_t serial1_index = 0; 
set_ranges_t tp_set_range;
int tp_assigned_val = -1;
String inputString = "";         // a String to hold incoming data
tp_ranges testpoint_ranges[8]; 

/* IMU project */
servos_t        servos; 
stream_mode_t   streaming; 
uint8_t         autonomy_test_init = 0; 
uint8_t         kinetic_test_init  = 0;  
state_servo_t   state_servo = NONE; 
orientation_t   orientation = CLKWISE ; 
uint8_t         counter; 
uint16_t         counter2; 
uint8_t         decode_string = 0; 

/* Proyect integration */
#define ELECTRIC_TEST 0
#define SENSOR_TEST   1
uint8_t device_testing_name = ELECTRIC_TEST; 
// uint8_t device_testing_name = ELECTRIC_TEST; 

/* Saving calibration on EEPROM */
calibrated_values_t calibrated_values; 
vref_t              Vref; 
const int           WRITTEN_SIGNATURE = 0xBEEFDEED;
int                 signature;
uint16_t            storedAddress = 0;
v_autocalib_t       vx; 

void setup() 
{
  init_vars(); 
  init_uart();
  init_adc();
}

void loop() 
{
    readSerial();
    
    // Testing FES
    if (device_testing_name == ELECTRIC_TEST){
        // Setting resistors mode 
        if (nresistor >= 0){
            setResistorValue(state, nresistor);
            nresistor = -1;
            state = STAND_BY;
        }

        // Setting testpoint mode 
        else if (tp_assigned_val >= 0){
            setTestpointValue(testpoint_ranges, state, tp_set_range, tp_assigned_val);
            tp_set_range = NOT_ASSIGNED; 
            tp_assigned_val = -1; 
            // state = STAND_BY;
        }

        // Run a test mode
        else if (tolerance > 0){
            int aux_tol = tolerance; 
            switch (state){
                case STAND_BY:
                    tolerance = 0;
                    break;
                case TP2:  // (A) A0 --> continuidad*
                    result_bat = testBattery(&calibrated_values, CH_tp2);
                    break;                    
                case TP9:  // (F) A1 --> ~2V
                    result_sbat = testSBat(&calibrated_values, CH_tp9);
                    break;
                case TP4:  // (B) A2 --> 3.3 V
                    result_3v3_TP4 = test3v3(&calibrated_values, CH_tp4);
                    break;
                case TP5:  // (C) A3 --> 3.3 V
                    result_3v3_TP5 = test3v3(&calibrated_values, CH_tp5);
                    break;
                case TP6:  // (D) A5 --> HV
                    result_HV = testHV(&calibrated_values, CH_tp6, connected_to_FES);
                    break;
                case TP10: // (G) A4 --> ~1.8V
                    result_pump = testPump(&calibrated_values, CH_tp10, connected_to_FES);                    
                    break;
                case TPALL: // (H)
                    testBattery(&calibrated_values, CH_tp2);
                    delay(10);
                    tolerance = aux_tol;

                    testSBat(&calibrated_values, CH_tp9);
                    delay(10);
                    tolerance = aux_tol; 
                    
                    test3v3(&calibrated_values, CH_tp4);
                    delay(10);
                    tolerance = aux_tol; 
                    
                    test3v3(&calibrated_values, CH_tp5);
                    delay(10);
                    tolerance = aux_tol; 
                    
                    testHV(&calibrated_values, CH_tp6, connected_to_FES);
                    delay(10);
                    tolerance = aux_tol; 
                    
                    result_pump = testPump(&calibrated_values, CH_tp10, connected_to_FES);   
                    aux_tol = 0; 
                    tolerance = 0; 
                    state = STAND_BY;                
                    break;
                case STIMULATE:
                    stimulate();
                    tolerance = 1; 
                    state = TP6;
                    break;
                case SERIALNUM:
                    serialNumber();
                    state = STAND_BY;
                    tolerance = 0; 
                    break; 
                case CALIBRATE:
                    autocalibration(&calibrated_values, &Vref, 0);
                    EEPROM.put(storedAddress + sizeof(signature), calibrated_values.voltage_divs);
                    EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values.voltage_divs) + sizeof(calibrated_values.V0), Vref);
                    state = STAND_BY;
                    tolerance = 0; 
                    break; 
                case CALIBRATE_HV:
                    if (connected_to_FES){
                        autocalibration_HV(&calibrated_values, &Vref, 0);
                        EEPROM.put(storedAddress + sizeof(signature), calibrated_values.voltage_divs);
                        EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values.voltage_divs) + sizeof(calibrated_values.V0), Vref);
                    }
                    else
                        SerialUSB.println("Connect to FES first");
                    state = STAND_BY;
                    tolerance = 0; 
                    break; 
                case RELAY1_ON:
                    SerialUSB.println("RELAY 5 ON");
                    digitalWrite(5, HIGH); // HIGH O LOW ?  
                    state = STAND_BY;
                    tolerance = 0; 
                    break; 
                case RELAY2_ON:
                    SerialUSB.println("RELAY 6 ON");
                    digitalWrite(6, HIGH); // HIGH O LOW ?  
                    state = STAND_BY;
                    tolerance = 0; 
                    break; 
                case RELAY_OFF:
                    SerialUSB.println("RELAYS OFF");
                    digitalWrite(5, LOW); // HIGH O LOW ? 
                    digitalWrite(6, LOW); // HIGH O LOW ? 
                    state = STAND_BY;
                    tolerance = 0; 
                    break; 
                case CONNECTING_BLE:
                    break; 
                case BLE_READY:
                    BLE_send_buffer();
                    state = STAND_BY;
                    break; 
                case BLE_COMMAND_FULL:
                    SerialUSB.println("Buffer lleno :(");
                    break; 
                case ERROR:
                    break;
                case RESET:
                    break;
                default:
                    SerialUSB.println("Default"); 
                    break;
            }
            state = STAND_BY;
            tolerance = 0;
            }
    }
    
    // Testing IMU
    else if (device_testing_name == SENSOR_TEST){
        switch(state){
            case KINETIC_TEST:  
                if (kinetic_test_init == 0)   run_kinetic_test();
                else    end_kinetic_test(); 
                break; 
            case AUTONOMY_TEST:  
                run_autonomy_test();
                autonomy_test_init = 1; 
                state = STAND_BY;   
                counter2 = 0; 
                break; 
            case END_AUTONOMY:  
                autonomy_test_init = 0; 
                state = STAND_BY;   
                break; 
            case BLE_READY:
                BLE_send_buffer();
                state = STAND_BY;
                break; 
            case STAND_BY: 
                counter2++; 
                if (kinetic_test_init == 1 and streaming == ON_stream and counter2 == 0){
                    if (decode_string){
                        decodeSensorStreamedData(serial1_RingBuffer_int); 
                        decode_string = 0; 
                    }
                    // counter  = counter  == 180  ? 0 : counter  + 5;
                    state_servo = kinetic_test_patron1(servos.servo1, servos.servo2, state_servo, 5);
                    if (state_servo == NONE){
                        SerialUSB.println("End streaming"); 
                        kinetic_test_init = 0; 
                        Serial1.println(">"); 
                        streaming = OFF_stream; 
                    }
                    else Serial1.println("S"); 
                }
                else if (autonomy_test_init == 1 and streaming == ON_stream and (counter2 % ((uint16_t) pow(2, 16)/4) == 1 or counter2 == 0)){
                    // if(streaming == ON_stream and (counter2 % ((uint16_t) pow(2, 16)/8) == 0 and counter2 >= (uint16_t) pow(2, 16)/8)){
                    if (decode_string){
                        decodeSensorStreamedData(serial1_RingBuffer_int); 
                        decode_string = 0; 
                    }

                    // streaming
                    if (counter2 == 0) Serial1.println("("); 
                    
                    // batería
                    else {
                        Serial1.println("S");
                        // SerialUSB.println(counter2);
                    }
                    delay(200); 
                    // state_servo = kinetic_test_patron1(servos.servo1, servos.servo2, state_servo, 5);
                    // if (state_servo == NONE){
                    //     SerialUSB.println("End streaming"); 
                    //     kinetic_test_init = 0; 
                    //     Serial1.println(">"); 
                    //     streaming = OFF_stream; 
                    // }

                    // else 
                }
                break;
            default:
                break; 
        }
    }
}

void readSerial(){

  // if (device_testing_name == ELECTRIC_TEST){
      // Find if ATSAM recieved data from BLE 
      while (Serial1.available()) {
          int data = Serial1.read();
          serial1_RingBuffer += (char) data;  
          serial1_RingBuffer_int[serial1_index] = data;  
          serial1_index++; 
          switch (data){
          case '=': // Se conectó un FES
              connected_to_FES = YES;
              break;
          case '#': // Se desconectó de FES
              connected_to_FES = NO;
              break;  
          case 0x80: 
              if ((kinetic_test_init == 1 or autonomy_test_init == 1) && streaming == OFF_stream){
                  streaming = PREPARE_stream; 
              }
              break; 
          case 0x12: 
              if ((kinetic_test_init == 1 or autonomy_test_init == 1) && streaming == PREPARE_stream){
                  streaming = ON_stream; 
              }
              break; 
          case '\n':
              if ((serial1_RingBuffer_int[0] != 'E') | serial1_RingBuffer_int[1] == 's'){
                  // if (serial1_RingBuffer_int[0] != '*') SerialUSB.print("*");
                  if (serial1_RingBuffer_int[0] == 0x80 && serial1_RingBuffer_int[1] == 0x12 && (kinetic_test_init == 1 or autonomy_test_init == 1)) {
                    decode_string = 1;  
                  }
                  else if (serial1_RingBuffer_int[0] == 0x80 && serial1_RingBuffer_int[1] == 0x12) decodeSensorStreamedData(serial1_RingBuffer_int); 
                  else {
                      if (serial1_RingBuffer.indexOf("OnConnect") > 0) {
                          connected_to_FES = YES;              
                      }
                      if (serial1_RingBuffer.indexOf("DisConnect") > 0) {
                          connected_to_FES = NO;              
                      }
                      if (serial1_RingBuffer.indexOf("*OnConnect") < 0 & serial1_RingBuffer.indexOf("*Bat") < 0)
                          SerialUSB.print(serial1_RingBuffer);
                      serial1_RingBuffer = "";
                  }
              } 
              serial1_index = 0; 
              break;
          default: 
              if (streaming == PREPARE_stream){
                  streaming = OFF_stream;
              }  
              break; 
        }
      }

      // Find if ATSAM recieved data from PC
      while (SerialUSB.available()) {
        int data = SerialUSB.read();

        // If 
        if (state != CONNECTING_BLE && state != BLE_READY){      
            if (setting_mode == TP_SETTING && tp_assigned_val == -1 && tp_set_range != NOT_ASSIGNED){
                serialUSB_RingBuffer += (char) data;
                if (serialUSB_RingBuffer.length() >= USB_RXBUFFER_SIZE - 1)
                {
                    state = BLE_COMMAND_FULL;
                    serialUSB_RingBuffer += '\n';
                } 
                else if (data == '-'){
                  tp_assigned_val = 0; 
                  serialUSB_RingBuffer = "";
                }
                else if (data == '\n') {
                    SerialUSB.println("End tp setting");
                    tp_assigned_val = serialUSB_RingBuffer.toInt();
                    serialUSB_RingBuffer = "";
                }
            }

            // Setting testpoint range mode on. Assign acceptable range to a testpoint. 
            else if (((state >= TP2 && state <= TPALL) || state == TP) && setting_mode == OFF){
                serialUSB_RingBuffer += (char) data;
                if (serialUSB_RingBuffer.length() >= USB_RXBUFFER_SIZE - 1){
                    SerialUSB.println("BLE COMMAND FULL"); 
                    state = BLE_COMMAND_FULL;
                    serialUSB_RingBuffer += '\n';
                }
                else if (data == '\n') {
                    tolerance = state == TP ? 1 : serialUSB_RingBuffer.toInt();
                    serialUSB_RingBuffer = "";
                }
            } 

            // Setting resistor value mode on. Assign value to resistor. 
            else if (setting_mode == RESISTOR_SETTING && state != STAND_BY && nresistor == -1){
                serialUSB_RingBuffer += (char) data;
                if (serialUSB_RingBuffer.length() >= USB_RXBUFFER_SIZE - 1)
                {
                    state = BLE_COMMAND_FULL;
                    serialUSB_RingBuffer += '\n';
                } 
                else if (data == '\n') {
                    nresistor = serialUSB_RingBuffer.toInt();
                    serialUSB_RingBuffer = "";
                }
            }

            // Setting VREF1 mode on. Assign value to resistor. 
            else if (setting_mode == VREF1_SETTING){
                serialUSB_RingBuffer += (char) data;
                if (serialUSB_RingBuffer.length() >= USB_RXBUFFER_SIZE - 1)
                {
                    state = BLE_COMMAND_FULL;
                    serialUSB_RingBuffer += '\n';
                    setting_mode = OFF; 
                } 
                else if (data == '\n') {
                    Vref.Vref1 = serialUSB_RingBuffer.toInt();
                    SerialUSB.print("Vref 1: ");
                    SerialUSB.println(Vref.Vref1);
                    EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values.voltage_divs) + sizeof(calibrated_values.V0), Vref);
                    serialUSB_RingBuffer = "";
                    setting_mode = OFF; 
                }
            }

            // Setting VREF2 mode on. Assign value to resistor. 
            else if (setting_mode == VREF2_SETTING){
                serialUSB_RingBuffer += (char) data;
                if (serialUSB_RingBuffer.length() >= USB_RXBUFFER_SIZE - 1)
                {
                    state = BLE_COMMAND_FULL;
                    serialUSB_RingBuffer += '\n';
                    setting_mode = OFF; 
                } 
                else if (data == '\n') {
                    Vref.Vref2 = serialUSB_RingBuffer.toInt();
                    SerialUSB.print("Vref 2: ");
                    SerialUSB.println(Vref.Vref2);
                    EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values.voltage_divs) + sizeof(calibrated_values.V0), Vref);
                    serialUSB_RingBuffer = "";
                    setting_mode = OFF; 
                }
            }
            
            // Setting VREFhv mode on. Assign value to resistor. 
            else if (setting_mode == VREFhv_SETTING){
                serialUSB_RingBuffer += (char) data;
                if (serialUSB_RingBuffer.length() >= USB_RXBUFFER_SIZE - 1)
                {
                    state = BLE_COMMAND_FULL;
                    serialUSB_RingBuffer += '\n';
                    setting_mode = OFF; 
                } 
                else if (data == '\n') {
                    Vref.Vrefhv = serialUSB_RingBuffer.toInt();
                    SerialUSB.print("Vref hv: ");
                    SerialUSB.println(Vref.Vrefhv);
                    EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values.voltage_divs) + sizeof(calibrated_values.V0), Vref);
                    serialUSB_RingBuffer = "";
                    setting_mode = OFF; 
                }
            }

            // Testing, selecting target type and selecting BLE, Settings and Testpoint modes 
            else if (setting_mode == OFF){
                switch(data){
                    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':  
                        // Testpoint measurement 
                        state = (state_t) (int) (data - 64);  
                        serialUSB_RingBuffer = ""; 
                        break;
                    case 'S':
                        // Testing stimulation (when BLE connected)
                        state = STIMULATE;
                        tolerance = 1; 
                        break;
                    case 'N':
                        // Testing serial number (when BLE connected) 
                        state = SERIALNUM;
                        tolerance = 1; 
                        break;
                    case '!':
                        // Selecting BLE mode
                        state = CONNECTING_BLE;
                        // serialUSB_RingBuffer += (char) data;
                        break;
                    case AUTOCALIBRATION:
                        state = CALIBRATE;
                        tolerance = 1; 
                        break;
                    case AUTOCALIBRATION_HV:
                        state = CALIBRATE_HV;
                        tolerance = 1; 
                        break;
                    // case READ_CALIB_VALS:
                    //     state = CALIB_VALS;
                    //     tolerance = 1; 
                    //     break;
                    case TEST_REF1:
                        state = RELAY1_ON;
                        tolerance = 1; 
                        break;
                    case TEST_REF2:
                        state = RELAY2_ON;
                        tolerance = 1; 
                        break;
                    case TEST_tps:
                        state = RELAY_OFF;
                        tolerance = 1; 
                        break;
                    case TESTPOINT:
                        // Selecting testpoint setting mode
                        SerialUSB.write('T');
                        setting_mode = TP_SETTING;
                        break; 
                    case SETTING_MODE: 
                        // Selecting resistors setting mode
                        SerialUSB.write('Z');
                        setting_mode = RESISTOR_SETTING; 
                        break; 
                    case SET_REF1: 
                        // Selecting Vref setting mode
                        setting_mode = VREF1_SETTING; 
                        break; 
                    case SET_REF2: 
                        // Selecting Vref setting mode
                        setting_mode = VREF2_SETTING; 
                        break; 
                    case SET_REFhv: 
                        // Selecting Vref setting mode
                        setting_mode = VREFhv_SETTING; 
                        break; 
                    case INPUT_FES:
                        // Selecting FES as target
                        SerialUSB.println("Modo FES");
                        target = FES;
                        break;
                    case START_KINETIC_TEST:
                        state = KINETIC_TEST; 
                        break;
                    case START_AUTONOMY_TEST:
                        // Selecting FES as target
                        SerialUSB.println("AUTONOMY TEST");
                        state = AUTONOMY_TEST; 
                        break;
                    case END_AUTONOMY_TEST:
                        // Selecting FES as target
                        SerialUSB.println("AUTONOMY TEST");
                        state = END_AUTONOMY; 
                        break;
                    case INPUT_WFES:
                        // Selecting WalkFES as target
                        SerialUSB.println("Modo WalkFES");
                        target = WALKFES;
                        break; 
                    case CHANGE_DEV_TO_IMU:
                        // Change device to IMU. Select Sensor Test
                        SerialUSB.println("IMU test");
                        Serial1.print("/\n");
                        device_testing_name = SENSOR_TEST;
                        break; 
                    case CHANGE_DEV_TO_FES:
                        // Change device to FES. Select Electric Test for FES
                        SerialUSB.println("FES test");
                        Serial1.print("-\n");
                        device_testing_name = ELECTRIC_TEST;
                        break; 
                    default: 
                        // Unknown mode
                        serialUSB_RingBuffer = ""; 
                        state = STAND_BY;
                        break;
                }
            }

            // In setting mode, find resistor number or number of testpoint
            else{
                switch(data){
                    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
                    case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
                    case 'U': case 'V': case 'W': case 'X': 
                        state = (state_t) data;  
                        serialUSB_RingBuffer = ""; 
                        break;  
                    case SET_LOWCUT:case SET_HIGHCUT: case SET_AVERAGE:
                        tp_set_range = (set_ranges_t) (data);
                        serialUSB_RingBuffer = ""; 
                        break; 
                    case END_SETTINGS_MODE: 
                        SerialUSB.println("Y");
                        // printTestpointRanges(testpoint_ranges);
                        setting_mode = OFF; 
                        state = STAND_BY; 
                        break; 
                    default:  
                        serialUSB_RingBuffer = ""; 
                        state = STAND_BY;
                        // tp_set_range = NOT_ASSIGNED;
                        break;
                }
            }
        }

        // Data recieved is being used for BLE connection    
        else{
          serialUSB_RingBuffer += (char) data;
          if (serialUSB_RingBuffer.length() >= USB_RXBUFFER_SIZE - 1){
            SerialUSB.println("BLE COMMAND FULL"); 
            state = BLE_COMMAND_FULL;
            serialUSB_RingBuffer += '\n';
          }
          else if (data == '\n') {
            state = BLE_READY;
            tolerance = 1;
          }
        }
      }
  // }
}












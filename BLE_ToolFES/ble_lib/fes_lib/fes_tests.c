#include "fes_tests.h"
#include "fes_common.h"
#include "string.h"

void highVoltageTest(volatile state_test_t * state_test, volatile bool * fes_adv){
    
    // PRINT_TO_TOOLFES("BLE: High voltage\r\n");

    // Control de Pin activación circuito de alto voltaje
    // uint8_t p_string[];
    // sprintf((char *) p_string, "_%s", "g1;1;20;20;4");

    // COMMAND_FES_BLE(p_string);
    COMMAND_FES_BLE("_g1;1;20;20;4");
    *state_test = NONE;
}


void serialNumber(volatile state_test_t * state_test, volatile bool * fes_adv){
    
    // PRINT_TO_TOOLFES("BLE: Serial Number\r\n");

    // Control de Pin para saber número de serie
    COMMAND_FES_BLE("_s");
    *state_test = NONE;
}
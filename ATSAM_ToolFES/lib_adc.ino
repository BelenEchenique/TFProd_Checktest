int sensorValue = 0;  // variable to store the value coming from the sensor

void init_adc() {
  analogReadResolution(12);
}

double ADCRead_OneChannel(nums CH_NUM)
{
    double mV = 0;
    for (uint16_t i=0; i < TEST_LENGTH; i++){
        mV += ((double) analogRead(CH_NUM));
        // mV += ((double) analogRead(CH_NUM)) * 1000 * 3.3/4096.0; // * 1000 * 3.3/256/256; 
        // delay(1); 
    }
    mV = (mV * 1000)/4096.0/TEST_LENGTH;
    return mV;
}

// Cambiar resistores
double ADCLib_ConvertValues( offset_voltage_t *v0, double mV_CH, nums CH_NUM){
    double real_mV;
    switch(CH_NUM){
        case CH_tp2: 
            real_mV = mV_CH <= 10 ? 0 : (mV_CH)*(R1 + R3)/R3;
            break;
        case CH_tp9: 
            real_mV = mV_CH <= 10 ? 0 : (mV_CH)*(R2 + R4)/R4;
            break; 
        case CH_tp4:
            real_mV = mV_CH <= 10 ? 0 : (mV_CH)*(R5 + R7)/R7;
            break;
        case CH_tp5: 
            real_mV = mV_CH <= 10 ? 0 : (mV_CH)*(R6 + R8)/R8;
            break;
        case CH_tp10: 
            real_mV = mV_CH <= 10 ? 0 : (mV_CH)*(R9 + R11)/R11;
            break;
        case CH_tp6: 
            // real_mV = (mV_CH)*(R10 + R12)/R12;
            real_mV = mV_CH <= 10 ? 0 : (mV_CH)*(R10 + R12)/R12;
            break;
        default: 
            real_mV = mV_CH;
            break;  
    }
    SerialUSB.print(real_mV/1000, 4); 
    return real_mV;
}

// 
double ADCLib_ConvertValues_Calibrated(voltage_divs_t *voltage_divs, offset_voltage_t *v0, double mV_CH, nums CH_NUM){
    double real_mV;
    switch(CH_NUM){
        case CH_tp2: 
            real_mV = mV_CH <= 10 ? 0 : (mV_CH)*(voltage_divs->VDIV_TP2) + v0->V0_TP2;
            break;
        case CH_tp4:
            real_mV = mV_CH <= 100 ? 0 : (mV_CH)*voltage_divs->VDIV_TP4 + v0->V0_TP4;
            break; 
        case CH_tp5: 
            real_mV = mV_CH <= 100 ? 0 : (mV_CH)*voltage_divs->VDIV_TP5 + v0->V0_TP5;
            break;
        case CH_tp6: 
            if (connected_to_FES){
                real_mV = mV_CH <= 1  ? 0 : (mV_CH)*voltage_divs->VDIV_TP6hv + v0->V0_TP6hv;
            }
            else{
                real_mV = mV_CH <= 1  ? 0 : (mV_CH)*voltage_divs->VDIV_TP6 + v0->V0_TP6;
            }
            break;
        case CH_tp9: 
            real_mV = mV_CH <= 100 ? 0 : (mV_CH)*voltage_divs->VDIV_TP9 + v0->V0_TP9;
            break;
        case CH_tp10: 
            real_mV = mV_CH <= 10 ? 0 : (mV_CH)*voltage_divs->VDIV_TP10 + v0->V0_TP10;
            break;
        default: 
            real_mV = mV_CH;
            break;  
    }
    SerialUSB.print(real_mV/1000, 3); 
    return real_mV;
}

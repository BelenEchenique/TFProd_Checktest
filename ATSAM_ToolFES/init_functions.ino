void getDefault_Tolerances(tol_config *config){
    config->TP1tolerance  = 0; 
    config->TP2tolerance  = 0; 
    config->TP3tolerance  = 0; 
    config->TP4tolerance  = 3; 
    config->TP5tolerance  = 3; 
    config->TP6tolerance  = 5; 
    config->TP7tolerance  = 6; 
    config->TP8tolerance  = 0; 
    config->TP9tolerance  = 5; 
    config->TP10tolerance = 5;  
}

void getDefault_ranges(tp_ranges *tps_ranges){
    // TP2
    tps_ranges[0].min_value = BATmin_default;   
    tps_ranges[0].max_value = BATmax_default;   
    tps_ranges[0].average_value = BATval_default;

    // TP4
    tps_ranges[1].min_value = TP4min_default;   
    tps_ranges[1].max_value = TP4max_default;   
    tps_ranges[1].average_value = TP4val_default;
    
    // TP5
    tps_ranges[2].min_value = TP5min_default;   
    tps_ranges[2].max_value = TP5max_default;   
    tps_ranges[2].average_value = TP5val_default;

    // TP6
    tps_ranges[3].min_value = TP6min_default;   
    tps_ranges[3].max_value = TP6max_default;   
    tps_ranges[3].average_value = TP6val_default;

    // TP6hv
    tps_ranges[4].min_value = TP6min_default_hv;   
    tps_ranges[4].max_value = TP6max_default_hv;   
    tps_ranges[4].average_value = TP6val_default_hv;

    // TP9
    tps_ranges[5].min_value = TP9min_default;   
    tps_ranges[5].max_value = TP9max_default;   
    tps_ranges[5].average_value = TP9val_default;

    // TP10
    tps_ranges[6].min_value = TP10min_default;   
    tps_ranges[6].max_value = TP10max_default;   
    tps_ranges[6].average_value = TP10val_default;

    // TP10hv
    tps_ranges[7].min_value = TP10min_default_hv;   
    tps_ranges[7].max_value = TP10max_default_hv;   
    tps_ranges[7].average_value = TP10val_default_hv;
}

void getDefault_Voltages(voltage_divs_t *voltage_divs, offset_voltage_t *v0, vref_t *Vref){
    read_calib_vals(0); 

    Vref->Vref1  = 999;    // Vref1 in mV used for autocalibration
    Vref->Vref2  = 2494;   // Vref2 in mV used for autocalibration
    Vref->Vrefhv = 174000; // Vrefhv in mV used for autocalibration
}

void printTestpointRanges(tp_ranges *tps_ranges){
    for (int i=0; i<8; i++){
        SerialUSB.print("TP");   
        SerialUSB.print(i);   
        SerialUSB.print(" -> min: ");   
        SerialUSB.print(tps_ranges[i].min_value);   
        SerialUSB.print(" max: ");   
        SerialUSB.print(tps_ranges[i].max_value);   
        SerialUSB.print(" average: ");
        SerialUSB.println(tps_ranges[i].average_value);
    }
}

void init_vars(void){
  batteryValue   = 0;
  target         = FES;
  result_bat     = 0;
  result_sbat    = 0;
  result_3v3_TP4 = 0; 
  result_3v3_TP5 = 0;
  result_HV      = 0; 
  result_pump    = 0; 
  result_stim    = 0; 
  
  getDefault_Tolerances(&tolerances);
  getDefault_ranges(testpoint_ranges);
  getDefault_Voltages(&calibrated_values.voltage_divs, &calibrated_values.V0, &Vref); 

  pinMode(PIN_RELAY1, OUTPUT);
  pinMode(PIN_RELAY2, OUTPUT);
  digitalWrite(PIN_RELAY1, LOW);
  digitalWrite(PIN_RELAY2, LOW);

  servos.servo1.attach(PIN_SERVO1);
  servos.servo2.attach(PIN_SERVO2);
}

void setTestpointValue(tp_ranges *tps_ranges, state_t testpoint, set_ranges_t set_range, int value){
    int aux = (int) testpoint - 64 == TP2 ? 0 : 
              (int) testpoint - 64 == TP4 ? 1 :
              (int) testpoint - 64 == TP5 ? 2 :
              (int) testpoint - 64 == TP6 ? 3 :
              (int) testpoint - 64 == TP6hv ? 4 :
              (int) testpoint - 64 == TP9 ? 5 :
              (int) testpoint - 64 == TP10 ? 6 :
              (int) testpoint - 64 == TP10hv ? 7 : 7;
    
    SerialUSB.print("aux: "); 
    SerialUSB.println(aux); 
    switch(set_range){
        case SET_LOWCUT:
            tps_ranges[aux].min_value = value; 
            break; 
        case SET_HIGHCUT:
            tps_ranges[aux].max_value = value;
            break; 
        case SET_AVERAGE: 
            tps_ranges[aux].average_value = value; 
            break; 
        default: 
            tps_ranges[aux].min_value = 0; 
            tps_ranges[aux].max_value = 0;
            tps_ranges[aux].average_value = 0; 
            break; 
    }
}

void setResistorValue(state_t resistor, int value){
    char buffer[20];
    switch (state)
    {
    case SET_R1:
        R1  = value;
        break; 
    case SET_R2:
        R2  = value;
        break; 
    case SET_R3:
        R3 = value;
        break; 
    case SET_R4:
        R4 = value;
        break; 
    case SET_R5:
        R5 = value;
        break; 
    case SET_R6:
        R6 = value;
        break; 
    case SET_R7:
        R7 = value;
        break; 
    case SET_R8:
        R8 = value;
        break; 
    case SET_R9:
        R9 = value;
        break; 
    case SET_R10:
        R10 = value;
        break; 
    case SET_R11:
        R11 = value;
        break; 
    case SET_R12:
        R12 = value;
        break;  
    default:
        break;
    }
}


void set_values(int hv){ 
    if (hv){
        TP10min = TP10min_default_hv; 
        TP10max = TP10max_default_hv; 
        TP10val = TP10val_default_hv;                           
        TP6min  = TP6min_default_hv; 
        TP6max  = TP6max_default_hv; 
        TP6val  = TP6val_default_hv;  
    }
    else 
        TP10min = TP10min_default; 
        TP10max = TP10max_default; 
        TP10val = TP10val_default;                           
        TP6min  = TP6min_default; 
        TP6max  = TP6max_default; 
        TP6val  = TP6val_default;  
}



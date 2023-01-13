char ADCbuff[11];

void toString(int32_t number){
    if (number > 0x80000000 && number > 0){
        SerialUSB.println("-");
        sprintf(ADCbuff, "%.4f", (0x100000000 - (number - 1))/1000);
        SerialUSB.println(ADCbuff);
        memset(ADCbuff, 0, 10);
    }
    else {
        sprintf(ADCbuff, "%.4f", number/1000);
        SerialUSB.println(ADCbuff);
        memset(ADCbuff, 0, 10);
        }
}

int evaluateTest(tp_ranges *tps_ranges, double realV, int CH_NUM){
    int result = REPROVED; 

    // Value is acceptable if it's between min_value and max_value
    if (tps_ranges[CH_NUM].min_value > 0 && tps_ranges[CH_NUM].max_value && tps_ranges[CH_NUM].average_value == 0){ 
        if (realV < tps_ranges[CH_NUM].max_value + tps_ranges[CH_NUM].max_value*tolerance/100 && realV > tps_ranges[CH_NUM].min_value - tps_ranges[CH_NUM].min_value*tolerance/100){
            SerialUSB.write(" B\r\n");
            tolerance = 0;  
            result = APROVED;
        }
        else{
            SerialUSB.write(" M\r\n");
            tolerance = 0;
            result = REPROVED; 
        }
    }

    // Value is acceptable if it's less than max_value
    else if (tps_ranges[CH_NUM].max_value > 0 && tps_ranges[CH_NUM].average_value == 0){
        if (realV < tps_ranges[CH_NUM].max_value + tps_ranges[CH_NUM].max_value*tolerance/100){
            SerialUSB.write(" B\r\n");
            tolerance = 0;  
            result =  APROVED;
        }
        else{
            SerialUSB.write(" M\r\n");
            tolerance = 0;
            result = REPROVED; 
        }
    }

    // Value is acceptable if it's greater than min_value
    else if (tps_ranges[CH_NUM].min_value > 0 && tps_ranges[CH_NUM].average_value == 0){
        if (realV > tps_ranges[CH_NUM].min_value - tps_ranges[CH_NUM].min_value*tolerance/100){
            SerialUSB.write(" B\r\n");
            tolerance = 0;  
            result = APROVED;
        }
        else{
            SerialUSB.write(" M\r\n");
            tolerance = 0;
            result = REPROVED; 
        }
    }

    // Value is acceptable if it's aprox. average_value
    else if (tps_ranges[CH_NUM].average_value > 0 && tps_ranges[CH_NUM].min_value == 0 && tps_ranges[CH_NUM].max_value == 0){
        if (realV > tps_ranges[CH_NUM].average_value - tps_ranges[CH_NUM].average_value*tolerance/100 && realV < tps_ranges[CH_NUM].average_value + tps_ranges[CH_NUM].average_value*tolerance/100){
            SerialUSB.write(" B\r\n");
            tolerance = 0;
            result = APROVED; 
        }
        else{
            SerialUSB.write(" M\r\n");
            tolerance = 0;
            result = REPROVED; 
        }
    }
    else{
      SerialUSB.write(" F\r\n");
      tolerance = 0;
      result = REPROVED; 
    }

    if (CH_NUM == 0 and result == APROVED){
        tps_ranges[5].average_value = realV/2; 
        tps_ranges[5].min_value = 0; 
        tps_ranges[5].max_value = 0; 
    }
    // SerialUSB.println(tps_ranges[CH_NUM].min_value);
    // SerialUSB.println(tps_ranges[CH_NUM].max_value);
    // SerialUSB.println(tps_ranges[CH_NUM].average_value);
    return result; 
}

/*******************************************************************************
 * Tests
 ******************************************************************************/

uint8_t autocalibration(calibrated_values_t *calibrated_values, vref_t *Vref, int print){
    // Ecuación para autocalibración : Vref = Vin*DV - Vo 
    //                                 Vref --> voltaje de referencia (de regulador)
    //                                 Vo   --> offset. Se mide antes de activar el relay
    //                                 Vin  --> voltaje leído en el ADC
    //                                 DV   --> divisor de voltaje    
    
    EEPROM.put(storedAddress, WRITTEN_SIGNATURE);

    digitalWrite(PIN_RELAY2, LOW); // HIGH O LOW ? 
    digitalWrite(PIN_RELAY1, HIGH); // HIGH O LOW ? 
    delay(100);

    vx.Vx_TP2.vx1  = ADCRead_OneChannel(CH_tp2); 
    vx.Vx_TP4.vx1  = ADCRead_OneChannel(CH_tp4); 
    vx.Vx_TP5.vx1  = ADCRead_OneChannel(CH_tp5); 
    vx.Vx_TP6.vx1  = ADCRead_OneChannel(CH_tp6); 
    vx.Vx_TP9.vx1  = ADCRead_OneChannel(CH_tp9); 
    vx.Vx_TP10.vx1 = ADCRead_OneChannel(CH_tp10); 

    digitalWrite(PIN_RELAY2, HIGH); // HIGH O LOW ? 
    delay(100);

    vx.Vx_TP2.vx2  = ADCRead_OneChannel(CH_tp2); 
    vx.Vx_TP4.vx2  = ADCRead_OneChannel(CH_tp4); 
    vx.Vx_TP5.vx2  = ADCRead_OneChannel(CH_tp5); 
    vx.Vx_TP6.vx2  = ADCRead_OneChannel(CH_tp6); 
    vx.Vx_TP6hv.vx2  = ADCRead_OneChannel(CH_tp6); 
    vx.Vx_TP9.vx2  = ADCRead_OneChannel(CH_tp9); 
    vx.Vx_TP10.vx2 = ADCRead_OneChannel(CH_tp10); 
    
    // Cálculo de pendiente en ecuación lineal y = mx - n --> m = (y2-y1)/(x2-x1) con 
    // y1 = VREF1,      y2 = VREF2
    // x1 = adc(VREF1), x2 = adc(VREF2) 
    calibrated_values->voltage_divs.VDIV_TP2 = (Vref->Vref2 - Vref->Vref1)/(vx.Vx_TP2.vx2 - vx.Vx_TP2.vx1);
    calibrated_values->voltage_divs.VDIV_TP4 = (Vref->Vref2 - Vref->Vref1)/(vx.Vx_TP4.vx2 - vx.Vx_TP4.vx1);
    calibrated_values->voltage_divs.VDIV_TP5 = (Vref->Vref2 - Vref->Vref1)/(vx.Vx_TP5.vx2 - vx.Vx_TP5.vx1);
    calibrated_values->voltage_divs.VDIV_TP6 = (Vref->Vref2 - Vref->Vref1)/(vx.Vx_TP6.vx2 - vx.Vx_TP6.vx1);
    calibrated_values->voltage_divs.VDIV_TP9 = (Vref->Vref2 - Vref->Vref1)/(vx.Vx_TP9.vx2 - vx.Vx_TP9.vx1);
    calibrated_values->voltage_divs.VDIV_TP10 = (Vref->Vref2 - Vref->Vref1)/(vx.Vx_TP10.vx2 - vx.Vx_TP10.vx1);
    calibrated_values->voltage_divs.VDIV_TP6hv = calibrated_values->voltage_divs.VDIV_TP6;
    
    calibrated_values->V0.V0_TP2 = Vref->Vref1 - calibrated_values->voltage_divs.VDIV_TP2 * vx.Vx_TP2.vx1;
    calibrated_values->V0.V0_TP4 = Vref->Vref1 - calibrated_values->voltage_divs.VDIV_TP4 * vx.Vx_TP4.vx1;
    calibrated_values->V0.V0_TP5 = Vref->Vref1 - calibrated_values->voltage_divs.VDIV_TP5 * vx.Vx_TP5.vx1;
    calibrated_values->V0.V0_TP6 = Vref->Vref1 - calibrated_values->voltage_divs.VDIV_TP6 * vx.Vx_TP6.vx1;
    calibrated_values->V0.V0_TP9 = Vref->Vref1 - calibrated_values->voltage_divs.VDIV_TP9 * vx.Vx_TP9.vx1;
    calibrated_values->V0.V0_TP10 = Vref->Vref1 - calibrated_values->voltage_divs.VDIV_TP10 * vx.Vx_TP10.vx1;
    calibrated_values->V0.V0_TP6hv = calibrated_values->V0.V0_TP6;

    digitalWrite(PIN_RELAY1, LOW); // HIGH O LOW ? 
    digitalWrite(PIN_RELAY2, LOW); // HIGH O LOW ? 

    if (print){
        SerialUSB.println("TP2 --> ");    
        SerialUSB.print("     n: ");  
        SerialUSB.println(calibrated_values->V0.V0_TP2); 
        SerialUSB.print("     m: "); 
        SerialUSB.println(calibrated_values->voltage_divs.VDIV_TP2); 
        
        SerialUSB.println("TP4 --> ");    
        SerialUSB.print("     n: ");  
        SerialUSB.println(calibrated_values->V0.V0_TP4); 
        SerialUSB.print("     m: "); 
        SerialUSB.println(calibrated_values->voltage_divs.VDIV_TP4); 
        
        SerialUSB.println("TP5 --> ");    
        SerialUSB.print("     n: ");  
        SerialUSB.println(calibrated_values->V0.V0_TP5); 
        SerialUSB.print("     m: "); 
        SerialUSB.println(calibrated_values->voltage_divs.VDIV_TP5); 
        
        SerialUSB.println("TP6 --> ");    
        SerialUSB.print("     n: ");  
        SerialUSB.println(calibrated_values->V0.V0_TP6); 
        SerialUSB.print("     m: "); 
        SerialUSB.println(calibrated_values->voltage_divs.VDIV_TP6); 
        
        SerialUSB.println("TP9 --> ");    
        SerialUSB.print("     n: ");  
        SerialUSB.println(calibrated_values->V0.V0_TP9); 
        SerialUSB.print("     m: "); 
        SerialUSB.println(calibrated_values->voltage_divs.VDIV_TP9); 
        
        SerialUSB.println("TP10 --> ");    
        SerialUSB.print("      n: ");  
        SerialUSB.println(calibrated_values->V0.V0_TP10); 
        SerialUSB.print("      m: "); 
        SerialUSB.println(calibrated_values->voltage_divs.VDIV_TP10); 
    }
    
    EEPROM.put(storedAddress + sizeof(signature), calibrated_values->voltage_divs);
    EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values->voltage_divs), calibrated_values->V0);
    EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values->voltage_divs) + sizeof(calibrated_values->V0), Vref);
    EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values->voltage_divs) + sizeof(calibrated_values->V0) + sizeof(Vref), vx);

    EEPROM.commit();
    SerialUSB.println("\nCalibration ready");
}

uint8_t autocalibration_HV(calibrated_values_t *calibrated_values, vref_t *Vref, int print){
    // Ecuación para autocalibración : Vref = Vin*DV - Vo 
    //                                 Vref --> voltaje de referencia (de regulador)
    //                                 Vo   --> offset. Se mide antes de activar el relay
    //                                 Vin  --> voltaje leído en el ADC
    //                                 DV   --> divisor de voltaje    
    
    EEPROM.put(storedAddress, WRITTEN_SIGNATURE);

    digitalWrite(PIN_RELAY2, LOW); // HIGH O LOW ? 
    digitalWrite(PIN_RELAY1, LOW); // HIGH O LOW ? 
    delay(5000);

    // vx.vx2  = ADCRead_OneChannel(CH_tp6); // v = 2.5 V
    // SerialUSB.println(vx.vx2);

    // digitalWrite(PIN_RELAY2, HIGH); // HIGH O LOW ? 
    // delay(5000);

    vx.Vx_TP6hv.vx1  = ADCRead_OneChannel(CH_tp6); // v = 170 V
    SerialUSB.println(vx.Vx_TP6hv.vx1);
    SerialUSB.println(vx.Vx_TP6hv.vx2);

    // Cálculo de pendiente en ecuación lineal y = mx - n --> m = (y2-y1)/(x2-x1) con 
    // y1 = VREFhv,      y2 = VREF2
    // x1 = adc(VREFhv), x2 = adc(VREF2) 
    calibrated_values->voltage_divs.VDIV_TP6hv = (Vref->Vref2 - Vref->Vrefhv)/(vx.Vx_TP6hv.vx2 - vx.Vx_TP6hv.vx1);
    calibrated_values->V0.V0_TP6hv = Vref->Vrefhv - calibrated_values->voltage_divs.VDIV_TP6hv * vx.Vx_TP6hv.vx1;
                                    
    if (print){
        SerialUSB.println("TP6 --> ");    
        SerialUSB.print("     n: ");  
        SerialUSB.println(calibrated_values->V0.V0_TP6); 
        SerialUSB.print("     m: "); 
        SerialUSB.println(calibrated_values->voltage_divs.VDIV_TP6); 
        
        SerialUSB.println("TP6hv --> ");    
        SerialUSB.print("     n: ");  
        SerialUSB.println(calibrated_values->V0.V0_TP6hv); 
        SerialUSB.print("     m: "); 
        SerialUSB.println(calibrated_values->voltage_divs.VDIV_TP6hv); 
    }
    
    EEPROM.put(storedAddress + sizeof(signature), calibrated_values->voltage_divs);
    EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values->voltage_divs), calibrated_values->V0);
    EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values->voltage_divs) + sizeof(calibrated_values->V0), Vref);
    EEPROM.put(storedAddress + sizeof(signature) + sizeof(calibrated_values->voltage_divs) + sizeof(calibrated_values->V0) + sizeof(Vref), vx);

    EEPROM.commit();
    SerialUSB.println("\nCalibration ready");
}

void read_calib_vals(int print){
    signature = 0; 
    EEPROM.get(storedAddress + sizeof(signature), calibrated_values.voltage_divs);
    EEPROM.get(storedAddress + sizeof(signature) + sizeof(calibrated_values.voltage_divs), calibrated_values.V0);
    EEPROM.get(storedAddress + sizeof(signature) + sizeof(calibrated_values.voltage_divs) + sizeof(calibrated_values.V0), Vref);

    if (print){
        SerialUSB.print("Vrefs:");
        SerialUSB.print("\tVref1: \t");
        SerialUSB.println(Vref.Vref1);
        SerialUSB.print("\tVref2: \t");
        SerialUSB.println(Vref.Vref2);

        SerialUSB.print("TP2:");
        SerialUSB.print("\tDivisor: \t");
        SerialUSB.println(calibrated_values.voltage_divs.VDIV_TP2);
        SerialUSB.print("\tOffset: \t");
        SerialUSB.println(calibrated_values.V0.V0_TP2, 10);

        SerialUSB.print("TP4:");
        SerialUSB.print("\tDivisor: \t");
        SerialUSB.println(calibrated_values.voltage_divs.VDIV_TP4);
        SerialUSB.print("\tOffset: \t");
        SerialUSB.println(calibrated_values.V0.V0_TP4, 10);
        
        SerialUSB.print("TP5:");
        SerialUSB.print("\tDivisor: \t");
        SerialUSB.println(calibrated_values.voltage_divs.VDIV_TP5);
        SerialUSB.print("\tOffset: \t");
        SerialUSB.println(calibrated_values.V0.V0_TP5, 10);
        
        SerialUSB.print("TP6:");
        SerialUSB.print("\tDivisor: \t");
        SerialUSB.println(calibrated_values.voltage_divs.VDIV_TP6);
        SerialUSB.print("\tOffset: \t");
        SerialUSB.println(calibrated_values.V0.V0_TP6, 10);
        
        SerialUSB.print("TP9:");
        SerialUSB.print("\tDivisor: \t");
        SerialUSB.println(calibrated_values.voltage_divs.VDIV_TP9);
        SerialUSB.print("\tOffset: \t");
        SerialUSB.println(calibrated_values.V0.V0_TP9, 10);
        
        SerialUSB.print("TP10:");
        SerialUSB.print("\tDivisor: \t");
        SerialUSB.println(calibrated_values.voltage_divs.VDIV_TP10);
        SerialUSB.print("\tOffset: \t");
        SerialUSB.println(calibrated_values.V0.V0_TP10, 10);
    }
    EEPROM.commit();
}

int testBattery(calibrated_values_t *calibrated_values, nums CH_NUM){
    double CHx, realV;
    SerialUSB.write(" TP2 ");

    if (tolerance >= 150){
        SerialUSB.write("N N\r\n");
        tolerance = 0;
        return REPROVED;
    } 

    if (tolerance >= 100){
        SerialUSB.write("F F\r\n");
        tolerance = 0;
        return REPROVED;
    } 

    CHx = ADCRead_OneChannel(CH_NUM); 
    realV = ADCLib_ConvertValues_Calibrated(&calibrated_values->voltage_divs, &calibrated_values->V0, CHx, CH_NUM);    

    state = STAND_BY;
    return evaluateTest(testpoint_ranges, realV, 0);
}

int test3v3(calibrated_values_t *calibrated_values, nums CH_NUM){
    double CHx, realV;

    if (CH_NUM == CH_tp4) {
        SerialUSB.write(" TP4 ");
    }
    else if (CH_NUM == CH_tp5) {
        SerialUSB.write(" TP5 ");
    }

    if (tolerance >= 150){
        SerialUSB.write("N N\r\n");
        tolerance = 0;
        return REPROVED;
    } 
     
    if (tolerance >= 100){
        SerialUSB.write("F F\r\n");
        tolerance = 0;
        return REPROVED;
    }     

    CHx = ADCRead_OneChannel(CH_NUM); 
    realV = ADCLib_ConvertValues_Calibrated(&calibrated_values->voltage_divs, &calibrated_values->V0, CHx, CH_NUM);    

    state = STAND_BY;

    if (CH_NUM == CH_tp4) return evaluateTest(testpoint_ranges, realV, 1);
    if (CH_NUM == CH_tp5) return evaluateTest(testpoint_ranges, realV, 2);
}

void stimulate(void){
    Serial1.println("}");
}

int testHV(calibrated_values_t *calibrated_values, nums CH_NUM, uint8_t FES_connected){
    double CHx, realV;
    int threshold;

    SerialUSB.write(" TP6 ");
    if (tolerance >= 150){
        SerialUSB.write("N N\r\n");
        tolerance = 0;
        return REPROVED;
    } 
    if (tolerance >= 100){
        SerialUSB.write("F F\r\n");
        tolerance = 0;
        return REPROVED;
    } 

    CHx = ADCRead_OneChannel(CH_NUM); 
    realV = ADCLib_ConvertValues_Calibrated(&calibrated_values->voltage_divs, &calibrated_values->V0, CHx, CH_NUM);    

    state = STAND_BY;

    if (FES_connected == NO){
        return evaluateTest(testpoint_ranges, realV, 3);
    }
    if (FES_connected == YES){
        return evaluateTest(testpoint_ranges, realV, 4);
    }
}

int testSBat(calibrated_values_t *calibrated_values, nums CH_NUM){
    double CHx, realV;

    SerialUSB.write(" TP9 ");
    if (tolerance >= 150){
        SerialUSB.write("N N\r\n");
        tolerance = 0;
        return REPROVED;
    } 
    if (tolerance >= 100){
        SerialUSB.write("F F\r\n");
        tolerance = 0;
        return REPROVED;
    } 

    CHx = ADCRead_OneChannel(CH_NUM); 
    realV = ADCLib_ConvertValues_Calibrated(&calibrated_values->voltage_divs, &calibrated_values->V0, CHx, CH_NUM);    

    state = STAND_BY;

    return evaluateTest(testpoint_ranges, realV,5);
}

int testPump(calibrated_values_t *calibrated_values, nums CH_NUM, uint8_t FES_connected){
    double CHx, realV;
    
    SerialUSB.write(" TPA ");
    if (tolerance >= 150){
        SerialUSB.write("N N\r\n");
        tolerance = 0;
        return REPROVED;
    } 
    if (tolerance >= 100){
        SerialUSB.write("F F\r\n");
        tolerance = 0;
        return REPROVED;
    } 

    CHx = ADCRead_OneChannel(CH_NUM); 
    realV = ADCLib_ConvertValues_Calibrated(&calibrated_values->voltage_divs, &calibrated_values->V0, CHx, CH_NUM);    

    state = STAND_BY;

    if (FES_connected == NO){
        return evaluateTest(testpoint_ranges, realV, 6);
    }
    if (FES_connected == YES){
        return evaluateTest(testpoint_ranges, realV, 7);
    }
}

void serialNumber(void){
    Serial1.write('&');   
    Serial1.write('\n');
    state = STAND_BY;
}

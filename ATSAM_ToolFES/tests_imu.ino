// #include "utils.h"

void move_servo(Servo servoMotor, orientation_t orientation, uint8_t delay_ms, uint8_t start, uint8_t end){
    uint8_t start_angle = start; 
    uint8_t end_angle   = end; 

    // Para el sentido positivo
    if (orientation == CLKWISE || orientation == BOTH){
        for (int i = start_angle; i <= end_angle; i++)  
        {
            // Desplazamos al ángulo correspondiente
            servoMotor.write(i);
            // Hacemos una pausa de 25ms
            delay(delay_ms);
        }
    }

    start_angle = orientation == BOTH ? end   : start; 
    end_angle   = orientation == BOTH ? start : end; 

    // Para el sentido negativo
    if (orientation == ANTICLKWISE || orientation == BOTH){
        for (int i = start_angle; i > end_angle; i--)
        {
            // Desplazamos al ángulo correspondiente
            servoMotor.write(i);
            // Hacemos una pausa de 25ms
            delay(delay_ms);
        }
    }
}

state_servo_t kinetic_test_patron1(Servo servo1, Servo servo2, state_servo_t state_servo, uint8_t vel){
    counter  = counter  == 180  ? 0 : counter  + vel;
    // counter2 = counter2 == 5   ? 0 : counter2 + 1;  
    
    switch (state_servo){
        case DIR1:
            servo1.write(counter);
            if (counter == 0) {
                // SerialUSB.println("1");
                return DIR2;
            }
            return DIR1; 
            break;
        case DIR2:  
            servo2.write(counter);
            if (counter == 0) {
                // SerialUSB.println("2");
                return DIR3;
            }
            return DIR2;  
            break;
        case DIR3:
            servo1.write(counter);
            if (counter == 0) {
                // SerialUSB.println("3");
                return DIR4;
            }
            return DIR3;  
            break;
        case DIR4:
            servo2.write(counter);
            if (counter == 0) {
                // SerialUSB.println("4");
                return DIR5;
            }
            return DIR4;  
            break;
        case DIR5:
            servo1.write(counter);
            if (counter == 0) {
                // SerialUSB.println("5");
                return DIR6;
            }
            return DIR5;  
            break;
        case DIR6:
            servo2.write(counter);
            if (counter == 0) {
                // SerialUSB.println("6");
                return NONE;
            }
            return DIR6;  
            break;
        default:
            // SerialUSB.println("default"); 
            return NONE;
            break;
        }
    return NONE; 
}

// uint8_t kinetic_test_patron2(app_pwm_t const * const PWM, state_servo_t state_servo){
//     uint8_t servo; 
//     switch (state_servo){
//         case DIR1:
//             servo = 0; 
//             move_servo_36(PWM, servo, FIRST, CLKWISE, 2);
//             // delay(500);
//             return DIR2;
//             break;
//         case DIR2:
//             servo = 1; 
//             move_servo_36(PWM, servo, FIRST, CLKWISE, 2);
//             // delay(500);
//             return DIR3;  
//             break;
//         case DIR3:
//             servo = 0; 
//             move_servo_36(PWM, servo, SECOND, CLKWISE, 2);
//             servo = 1; 
//             move_servo_36(PWM, servo, SECOND, CLKWISE, 2);
//             // delay(500);
//             return DIR4;  
//             break;
//         case DIR4:
//             servo = 1; 
//             move_servo_36(PWM, servo, THIRD, CLKWISE, 2);
//             // delay(500);
//             return DIR5;  
//             break;
//         case DIR5:
//             servo = 0; 
//             move_servo_36(PWM, servo, THIRD, CLKWISE, 2);
//             // delay(500);
//             return DIR6;  
//             break;
//         case DIR6:
//             servo = 1; 
//             move_servo_36(PWM, servo, FOURTH, CLKWISE, 2);
//             servo = 0; 
//             move_servo_36(PWM, servo, FOURTH, CLKWISE, 2);
//             // delay(500);
//             return DIR7;  
//             break;
//         case DIR7:
//             servo = 1; 
//             move_servo_36(PWM, servo, FIFTH, CLKWISE, 2);
//             // delay(500);
//             return DIR8;  
//             break;
//         case DIR8:
//             servo = 0; 
//             move_servo_36(PWM, servo, FIFTH, CLKWISE, 2);
//             // delay(500);
//             return DIR9;  
//             break;
//         case DIR9:
//             servo = 0; 
//             move_servo_180(PWM, servo, ANTICLKWISE, 25);
//             // delay(500);
//             return DIR10;  
//             break;
//         case DIR10:
//             servo = 1; 
//             move_servo_180(PWM, servo, ANTICLKWISE, 25);
//             delay(500);
//             // app_pwm_disable(&PWM);
//             return DIR1;  
//             break;
//         default:
//             state_servo = DIR1;
//             break;
//         }
//     return 0; 
// }

// uint8_t kinetic_test_patron3(app_pwm_t const * const PWM, state_servo_t state_servo){
//     uint8_t servo; 
//     switch (state_servo){
//         case DIR1:
//             servo = 0; 
//             move_servo_36(PWM, servo, FIRST, CLKWISE, 2);
//             // delay(500);
//             return DIR2;
//             break;
//         case DIR2:
//             servo = 1; 
//             move_servo_36(PWM, servo, FIRST, CLKWISE, 2);
//             // delay(500);
//             return DIR3;  
//             break;
//         case DIR3:
//             servo = 0; 
//             move_servo_36(PWM, servo, SECOND, CLKWISE, 2);
//             servo = 1; 
//             move_servo_36(PWM, servo, SECOND, CLKWISE, 2);
//             // delay(500);
//             return DIR4;  
//             break;
//         case DIR4:
//             servo = 1; 
//             move_servo_36(PWM, servo, THIRD, CLKWISE, 2);
//             // delay(500);
//             return DIR5;  
//             break;
//         case DIR5:
//             servo = 0; 
//             move_servo_36(PWM, servo, THIRD, CLKWISE, 2);
//             // delay(500);
//             return DIR6;  
//             break;
//         case DIR6:
//             servo = 1; 
//             move_servo_36(PWM, servo, FOURTH, CLKWISE, 2);
//             servo = 0; 
//             move_servo_36(PWM, servo, FOURTH, CLKWISE, 2);
//             // delay(500);
//             return DIR7;  
//             break;
//         case DIR7:
//             servo = 1; 
//             move_servo_36(PWM, servo, FIFTH, CLKWISE, 2);
//             // delay(500);
//             return DIR8;  
//             break;
//         case DIR8:
//             servo = 0; 
//             move_servo_36(PWM, servo, FIFTH, CLKWISE, 2);
//             // delay(500);
//             return DIR9;  
//             break;
//         case DIR9:
//             servo = 0; 
//             move_servo_180(PWM, servo, ANTICLKWISE, 25);
//             // delay(500);
//             return DIR10;  
//             break;
//         case DIR10:
//             servo = 1; 
//             move_servo_180(PWM, servo, ANTICLKWISE, 25);
//             delay(500);
//             // app_pwm_disable(&PWM);
//             return DIR1;  
//             break;
//         default:
//             state_servo = DIR1;
//             break;
//         }
//     return 0; 
// }

void run_kinetic_test(){  
    Serial1.println("<"); 
    state = STAND_BY;
    kinetic_test_init = 1; 
    state_servo = DIR1; 
    counter2 = 0; 
}

void end_kinetic_test(){  
    kinetic_test_init = 0; 
    state_servo = DIR1; 
    state = STAND_BY;   
    counter2 = 0; 
    Serial1.println(">"); 
    streaming = OFF_stream; 
}

void run_autonomy_test(){  
    Serial1.println("%"); 
}

// void start_stream_mode(){
//     Serial1.print("#\n"); 
//     delay(1000); 
// }

// void end_stream_mode(){
//     Serial1.print("$\n"); 
//     delay(500); 
// }

// typedef struct sensor_data{
//     uint8_t   index_data_type;
//     uint32_t  data_sensor[SENSOR_PACKAGES];
//     long double  data_decoded[10];
// } sensor_data_t;


void decodeSensorStreamedData(uint8_t *received_data){

    // Programar esta función
    qf16 variable_reception;
    char string[40];  

    memcpy(&variable_reception.a, &received_data[2], 4);
    memcpy(&variable_reception.b, &received_data[6], 4);
    memcpy(&variable_reception.c, &received_data[10], 4);
    memcpy(&variable_reception.d, &received_data[14], 4);

    float w = ((float) variable_reception.a) / fix16_one;  
    float x = ((float) variable_reception.b) / fix16_one; 
    float y = ((float) variable_reception.c) / fix16_one; 
    float z = ((float) variable_reception.d) / fix16_one; 

    float yaw   = atan2(2 * (w * z + x * y), 1 - 2 * (y * y + z * z));
    float pitch = abs(2.0f * (w * y - z * x)) >= 1 ? copysign(M_PI / 2, 2.0f * (w * y - z * x)) : asin(2.0f * (w * y - z * x));
    float roll = atan2(2.0f * (w * x + y * z), 1 - 2.0f * (x * x + y * y));
    
    sprintf(string, "%0.4f;%0.4f;%0.4f;", roll, pitch, yaw);
    // sprintf(string, "%0.4f;%0.4f;%0.4f;%0.4f;", w, x, y, z);

    SerialUSB.println(string); 
    serial1_RingBuffer = ""; 
}
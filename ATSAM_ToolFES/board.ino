#define fix16_one      65536

/* Voltajes esperados en diferentes test points */
#define BATmax_default 4200    // Valor máximo esperado para la batería
#define BATmin_default 3300    // Valor mínimo esperado para la batería
#define BATval_default 0       // Valor mínimo esperado para la batería
#define TP4max_default 0       // Valor esperado en TP4
#define TP4min_default 0       // Valor esperado en TP4
#define TP4val_default 3300U   // Valor esperado en TP4
#define TP5max_default 0       // Valor esperado en TP5
#define TP5min_default 0       // Valor esperado en TP5
#define TP5val_default 3300U   // Valor esperado en TP5

#define TP6min_default 3500    // Valor esperado en TP6
#define TP6max_default 3900    // Valor esperado en TP6
#define TP6val_default 0       // Valor esperado en TP6

#define TP6min_default_hv 0       // Valor esperado en TP6
#define TP6max_default_hv 0       // Valor esperado en TP6
#define TP6val_default_hv 170000  // Valor esperado en TP6

#define TP9min_default 0       // Valor esperado en TP9
#define TP9max_default 0       // Valor esperado en TP9
#define TP9val_default 2000U   // Valor esperado en TP9

#define TP10min_default 1700   // Valor esperado en TP10
#define TP10max_default 1900   // Valor esperado en TP10
#define TP10val_default 0      // Valor esperado en TP10

#define TP10min_default_hv 0      // Valor esperado en TP10
#define TP10max_default_hv 0      // Valor esperado en TP10
#define TP10val_default_hv 6000U  // Valor esperado en TP10

/* Voltajes esperados en diferentes test points */
#define FES           1U
#define WALKFES       2U
#define TESTPOINT1    'A'             // TP n°1 
#define TESTPOINT2    'B'             // TP n°2
#define TESTPOINT3    'C'             // TP n°3 
#define TESTPOINT4    'D'             // TP n°4
#define TESTPOINT5    'E'             // TP n°5
#define TESTPOINT6    'F'             // TP n°6
#define TESTPOINT7    'G'             // TP n°7
#define TESTPOINT8    'H'             // TP n°8
#define TESTPOINT9    'I'             // TP n°9
#define TESTPOINT10   'J'             // TP n°10

/* Resultados tests */
#define APROVED       1
#define REPROVED      0
#define TEST_LENGTH   2000U

/* PINS */
#define PIN_RELAY1 5  // Control Relés
#define PIN_RELAY2 6  // Control Relés
#define PIN_RELAY3 7  // Control Relés
#define PIN_SERVO1 9  // PWM para Servos
#define PIN_SERVO2 10 // PWM para Servos

volatile int32_t batteryValue;
tol_config tolerances;  

volatile uint32_t BATmax = BATmax_default;
volatile uint32_t BATmin = BATmin_default;
volatile uint32_t BATval = BATval_default;
volatile uint32_t TP4max = TP4max_default;
volatile uint32_t TP4min = TP4min_default;
volatile uint32_t TP4val = TP4val_default;
volatile uint32_t TP5max = TP5max_default;
volatile uint32_t TP5min = TP5min_default;
volatile uint32_t TP5val = TP5val_default;

volatile uint32_t TP6min = TP6min_default;
volatile uint32_t TP6max = TP6max_default;
volatile uint32_t TP6val = TP6val_default;

volatile uint32_t TP9min = TP9min_default;
volatile uint32_t TP9max = TP9max_default;
volatile uint32_t TP9val = TP9val_default;

volatile uint32_t TP10min = TP10min_default;
volatile uint32_t TP10max = TP10max_default;
volatile uint32_t TP10val = TP10val_default;

volatile uint16_t txIndex; /* Index of the data to send out. */
volatile uint16_t rxIndex; /* Index of the memory to save new arrived data. */

typedef struct {
    fix16_t a; // Real part
    fix16_t b; // i
    fix16_t c; // j
    fix16_t d; // k
} qf16;

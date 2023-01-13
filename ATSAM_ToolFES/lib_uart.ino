/* UART instance */
#define UART1_IRQHandler        UART1_RX_TX_IRQHandler
#define UART0_IRQHandler        UART0_RX_TX_IRQHandler

char FESbuffer[USB_RXBUFFER_SIZE] = "";
volatile bool flag = false;

bool stringComplete = false;  // whether the string is complete

void init_uart(void){
  SerialUSB.begin(460800);
  Serial1.begin(115200);
  inputString.reserve(BLE_RXBUFFER_SIZE);
  serial1_RingBuffer.reserve(BLE_RXBUFFER_SIZE);
  serialUSB_RingBuffer.reserve(USB_RXBUFFER_SIZE-1);
}

void BLE_send_buffer(){ 
  if (serialUSB_RingBuffer[0] >  '0' && serialUSB_RingBuffer[0] <= '9'){
      Serial1.print('!'); 
  } 
  Serial1.print(serialUSB_RingBuffer);
  serialUSB_RingBuffer = ""; 
}
 
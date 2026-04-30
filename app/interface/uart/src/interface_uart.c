#include "interface_uart.h"
#include "stm32g4xx_hal.h"



#define UART_TX_TIMEOUT_MS 50U

#define UART_TX_DMA_BUF_SIZE (UART_PACKET_SIZE * TIMER_COUNT)

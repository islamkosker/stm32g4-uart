/**
 * @file protocol_uart_tx_stub.c
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-05 22:49
 * @version 1.0.0
 * @copyright Copyright (c) 2026 İslam Köşker.
 */
#include "app_protocol.h"
#include "interface_uart.h"
#include "string.h"


uint8_t last_tx[UART_PACKET_SIZE] = {0};
int tx_count = 0;

bool interface_uart_send(const uint8_t* data, size_t len)
{
    if (len == UART_PACKET_SIZE)
    {
        memcpy(last_tx, data, len);
        tx_count++;
    }
    return true;
}
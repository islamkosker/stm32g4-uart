/**
 * @file interface_uart.h
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-04-28 22:10
 * @version 1.0.0
 * @copyright Copyright (c) 2026 İslam Köşker. All rights reserved.
 */

#ifndef INTERFACE_UART_H
#define INTERFACE_UART_H

#include <stddef.h>
#include <stdint.h>

void interface_uart_send(const uint8_t, size_t len);
void interface_uart_poll_tx(void);

#endif
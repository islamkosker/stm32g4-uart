/**
 * @file app.c
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-05 19:31
 * @version 1.0.0
 * @copyright Copyright (c) 2026 İslam Köşker.
 */

#include "app_protocol.h"
#include "interface_uart.h"
extern uart_parser_t g_parser;

void app(void)
{
    uart_parser_init(&g_parser);
    while (1)
    {
        interface_uart_poll_tx();
    }
}
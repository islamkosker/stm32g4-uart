/**
 * @file app_protocol.h
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-04-28 21:44
 * @version 1.0.0
 * @copyright Copyright (c) 2026 İslam Köşker. All rights reserved.
 */

#ifndef APP_PROTOCOL_H
#define APP_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define UART_SOF 0xAAU
#define UART_PACKET_SIZE 5

typedef enum
{
    CMD_GET_STATUS = 0,
} uart_cmd_t;

typedef enum
{
    PARSE_WAIT_SOF = 0,
    PARSE_CMD,
    PARSE_P_HI, // payload high
    PARSE_P_LO, // payload low
    PARSE_CRC
} parse_state_t;

typedef struct
{
    parse_state_t state;
    uint8_t cmd;
    uint8_t p_hi;
    uint8_t p_lo;
    uint8_t crc;
} uart_parser_t;

void uart_parser_init(uart_parser_t* parser);
void uart_dispatcher(uart_parser_t* parser);
bool uart_parser_check(uart_parser_t* parser, uint8_t byte);

void app_uart_feed(const uint8_t *data, uint16_t len);


#endif // APP_PROTOCOL_H
/**
 * @file app_protocol.c
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-05 20:14
 * @version 1.0.0
 * @copyright Copyright (c) 2026 İslam Köşker.
 */

#include "app_protocol.h"
#include "interface_uart.h"

#define UINT8_BIT 8
#define CRC8_MSB_MASK (1u << (UINT8_BIT - 1))
#define CRC8_POLY 0x07

uart_parser_t g_parser = {0};

static uint8_t crc_u8(uint8_t crc, uint8_t data)
{
    crc ^= data;
    for (size_t bit = 0; bit < UINT8_BIT; bit++)
        crc = (uint8_t)(crc & CRC8_MSB_MASK) ? (crc << 1) ^ CRC8_POLY : (crc << 1);

    return crc;
}

static uint8_t crc_block(uint8_t crc, const uint8_t* data, size_t n)
{
    for (size_t i = 0; i < n; i++)
        crc = crc_u8(crc, data[i]);
    return crc;
}

static bool check_packet_wait_sof(uart_parser_t* parser, uint8_t byte)
{
    if (byte != UART_SOF) return false;

    parser->crc = crc_u8(0x00, byte);
    parser->state = PARSE_CMD;
    return false;
}

static bool check_packet_cmd(uart_parser_t* parser, uint8_t byte)
{
    parser->cmd = byte;
    parser->crc = crc_u8(parser->crc, byte);
    parser->state = PARSE_P_HI;
    return false;
}

static bool check_packet_payload_high(uart_parser_t* parser, uint8_t byte)
{
    parser->p_hi = byte;
    parser->crc = crc_u8(parser->crc, byte);
    parser->state = PARSE_P_LO;
    return false;
}

static bool check_packet_payload_low(uart_parser_t* parser, uint8_t byte)
{
    parser->p_lo = byte;
    parser->crc = crc_u8(parser->crc, byte);
    parser->state = PARSE_CRC;
    return false;
}

static bool check_packet_crc(uart_parser_t* parser, uint8_t byte)
{
    if (byte == parser->crc)
    {
        parser->state = PARSE_WAIT_SOF;
        return true;
    }
    else if (byte == UART_SOF)
    {
        parser->crc = crc_u8(0x00, byte);
        parser->state = PARSE_CMD;
        return false;
    }

    parser->state = PARSE_WAIT_SOF;
    return false;
}

void app_uart_feed(const uint8_t* data, uint16_t len)
{
    if (!data || !len) return;

    for (uint16_t i = 0U; i < len; i++)
        if (uart_parser_check(&g_parser, data[i])) uart_dispatcher(&g_parser);
}

static void uart_get_status(uart_parser_t* parser)
{
    // example impl.
    uint8_t frame[UART_PACKET_SIZE] = {0};

    frame[0] = UART_SOF;
    frame[1] = CMD_GET_STATUS;
    frame[2] = parser->p_hi;
    frame[3] = parser->p_lo;
    frame[4] = crc_block(0, frame, 4);

    (void)interface_uart_send(frame, UART_PACKET_SIZE);
}

void uart_parser_init(uart_parser_t* parser)
{
    parser->state = PARSE_WAIT_SOF;
    parser->crc = 0;
}

bool uart_parser_check(uart_parser_t* parser, uint8_t byte)
{
    switch (parser->state)
    {
    case PARSE_WAIT_SOF: return check_packet_wait_sof(parser, byte);
    case PARSE_CMD: return check_packet_cmd(parser, byte);
    case PARSE_P_HI: return check_packet_payload_high(parser, byte);
    case PARSE_P_LO: return check_packet_payload_low(parser, byte);
    case PARSE_CRC: return check_packet_crc(parser, byte);
    default: return false;
    }
}

void uart_dispatcher(uart_parser_t* parser)
{
    switch (parser->cmd)
    {
    case CMD_GET_STATUS: uart_get_status(parser); break;
    }
}
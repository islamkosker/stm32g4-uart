/**
 * @file test_protocol.c
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-05 21:09
 * @version 1.0.0
 * @copyright Copyright (c) 2026 İslam Köşker.
 */
#include "app_protocol.h"
#include "unity.h"

#define UART_SOF 0xAAU
#define CRC8_POLY 0x07U

extern int tx_count;
extern uint8_t last_tx[];

static uint8_t crc8(const uint8_t* data, size_t n)
{
    uint8_t crc = 0;
    for (size_t i = 0; i < n; ++i)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b)
            crc = (crc & 0x80U) ? (uint8_t)((crc << 1) ^ CRC8_POLY) : (uint8_t)(crc << 1);
    }
    return crc;
}

void test_crc_error__followed_by_valid_packet__only_valid_packet_dispatched(void)
{
    uart_parser_t parser;
    uart_parser_init(&parser);

    tx_count = 0;

    uint8_t stream[] = {UART_SOF, CMD_GET_STATUS, 0x00,
                        100, // CRC missing packet

                        UART_SOF, CMD_GET_STATUS, 0x00, 200, 0};

    stream[8] = crc8(&stream[4], 4);
    for (size_t i = 0; i < sizeof(stream); i++)
    {
        if (uart_parser_check(&parser, stream[i])) uart_dispatcher(&parser);
    }

    TEST_ASSERT_EQUAL_INT(1, tx_count);
    TEST_ASSERT_EQUAL_INT(UART_SOF, last_tx[0]);
    TEST_ASSERT_EQUAL_INT(stream[5], last_tx[1]);
    TEST_ASSERT_EQUAL_INT(stream[6], last_tx[2]);
    TEST_ASSERT_EQUAL_INT(stream[7], last_tx[3]);

    uint8_t expected_crc = crc8(last_tx, 4);
    TEST_ASSERT_EQUAL_INT(parser.crc, expected_crc);
}

void test_two_valid_packets__received_back_to_back__both_dispatched(void)
{
    uart_parser_t parser;
    uart_parser_init(&parser);

    tx_count = 0;

    uint8_t stream[10];

    stream[0] = UART_SOF;
    stream[1] = CMD_GET_STATUS;
    stream[2] = 1;
    stream[3] = 2;
    stream[4] = crc8(stream, 4);

    stream[5] = UART_SOF;
    stream[6] = CMD_GET_STATUS;
    stream[7] = 3;
    stream[8] = 4;
    stream[9] = crc8(&stream[5], 4);

    for (int i = 0; i < 10; i++)
    {
        if (uart_parser_check(&parser, stream[i])) uart_dispatcher(&parser);
    }

    TEST_ASSERT_EQUAL_INT(2, tx_count);
}

void test_crc_error__new_sof_received__parser_resyncs_and_parses_next_packet(void)
{
    uart_parser_t parser;
    uart_parser_init(&parser);

    tx_count = 0;

    uint8_t stream[] = {
        UART_SOF, CMD_GET_STATUS, UART_SOF, 0x01, 0xFF,
        UART_SOF, CMD_GET_STATUS, 0x02, 0x03, 0x00
    };

    stream[9] = crc8(&stream[5], 4);

    for (size_t i = 0; i < sizeof(stream); i++)
    {
        if (uart_parser_check(&parser, stream[i]))
            uart_dispatcher(&parser);
    }

    TEST_ASSERT_EQUAL_INT(1, tx_count);
}

void test_protocol(void)
{
    RUN_TEST(test_crc_error__followed_by_valid_packet__only_valid_packet_dispatched);
    RUN_TEST(test_two_valid_packets__received_back_to_back__both_dispatched);
    RUN_TEST(test_crc_error__new_sof_received__parser_resyncs_and_parses_next_packet);
}
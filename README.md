# stm32g4-uart

Bare-metal STM32G4 UART driver with DMA, non-blocking TX queue, and a lightweight binary protocol parser.

---

## Motivation

CubeMX generates a `main.c` that tightly couples peripheral initialization, application logic, and HAL boilerplate into a single file. This makes the codebase hard to maintain and test as the project grows.

This project adopts a clean separation: CubeMX-generated code is left untouched. The only manual edit to `main.c` is a single `app()` call inside the `while(1)` loop. All application logic lives in the `app/` directory.

```c
/* main.c — only this line is added manually */
int main(void)
{
    //....
    /* USER CODE BEGIN 2 */
    app();  /* <-- single entry point into application code */
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
```

---

## Project Structure

```
app/
├── CMakeLists.txt
│
├── interface/
│   └── uart/
│       ├── inc/
│       │   └── interface_uart.h
│       └── src/
│           └── interface_uart.c      # DMA UART driver
│
├── main/
│   └── app.c                         # Application entry point
│
└── protocol/
    ├── inc/
    │   └── app_protocol.h
    └── src/
        └── app_protocol.c            # Binary protocol parser & dispatcher
```

### Layer Responsibilities

| Layer       | File               | Responsibility                                   |
| ----------- | ------------------ | ------------------------------------------------ |
| Interface   | `interface_uart.c` | DMA RX/TX, TX queue, HAL callbacks               |
| Protocol    | `app_protocol.c`   | Packet parsing, CRC validation, command dispatch |
| Application | `app.c`            | Poll loop, application logic                     |

---

## UART Driver (`interface_uart`)

### Features

- **Non-blocking DMA TX** via a circular queue — `interface_uart_send()` never blocks the caller
- **TX queue** with 4 fixed-size slots (`UART_TX_QUEUE_SIZE`), power-of-2 indexed for efficiency
- **TX timeout protection** — if `TxCpltCallback` never fires, the driver self-recovers after `UART_TX_TIMEOUT_MS`
- **DMA RX with idle detection** via `HAL_UARTEx_ReceiveToIdle_DMA` — handles variable-length packets and line-idle events
- **Auto-restart on error** — `HAL_UART_ErrorCallback` clears flags and restarts DMA receive

### TX Data Flow

```
interface_uart_send()     →  enqueue packet into tx_queue[]
interface_uart_poll_tx()  →  if IDLE and queue non-empty: start DMA TX
HAL_UART_TxCpltCallback() →  advance head, decrement count, set IDLE
```

### RX Data Flow

```
DMA fills uart_rx_dma_buffer[]
HAL_UARTEx_RxEventCallback() →  app_uart_feed(buffer, size)
                              →  restart DMA receive
```

### Configuration

Defined in `interface_uart.h`:

```c
#define UART_PACKET_SIZE     5    // Fixed packet length in bytes
#define UART_TX_QUEUE_SIZE   4    // Must be a power of 2
```

To retarget to a different UART peripheral, change only this line in `interface_uart.c`:

```c
#define INTERFACE_UART hlpuart1   // replace with e.g. huart2
```

---

## Protocol (`app_protocol`)

### Packet Format

| Byte | Field | Description                           |
| ---- | ----- | ------------------------------------- |
| 0    | SOF   | Start of frame (`0xAA` or as defined) |
| 1    | CMD   | Command identifier                    |
| 2    | P_HI  | Payload high byte                     |
| 3    | P_LO  | Payload low byte                      |
| 4    | CRC   | CRC-8 over bytes 0–3                  |

Total packet size: **5 bytes** (`UART_PACKET_SIZE`)

### CRC

CRC-8 with polynomial `0x07`, initial value `0x00`, computed over all bytes preceding the CRC field.

### Parser State Machine

```
WAIT_SOF → CMD → P_HI → P_LO → CRC
                                  │
                    valid CRC ────┤──→ uart_dispatcher()
                  invalid CRC ────┘──→ WAIT_SOF
```

The parser is byte-driven and stateless between calls — safe to feed from DMA callbacks.

### Adding a New Command

1. Add a new `CMD_*` constant to `app_protocol.h`
2. Add a handler function in `app_protocol.c`
3. Add a `case` to `uart_dispatcher()`

```c
// app_protocol.c
static void uart_handle_my_cmd(uart_parser_t* parser)
{
    // build and send response
}

void uart_dispatcher(uart_parser_t* parser)
{
    switch (parser->cmd)
    {
    case CMD_GET_STATUS: uart_get_status(parser); break;
    case CMD_MY_CMD:     uart_handle_my_cmd(parser); break;
    }
}
```

---

## Build

CMake with a Ninja/Make backend and an ARM GCC toolchain.

```bash
# Configure
cmake --preset Debug

# Build
cmake --build --preset Debug
```

### Memory Usage (STM32G431 — 32 KB RAM / 128 KB Flash)

```
RAM:   ~2.0 KB / 32 KB   (6%)
FLASH: ~18.0 KB / 128 KB (14%)
```

The majority of FLASH is consumed by the STM32 HAL. Application code footprint is minimal.

---

## Target

| Field        | Value                            |
| ------------ | -------------------------------- |
| MCU          | STM32G431                        |
| HAL          | STM32G4xx HAL (CubeMX generated) |
| UART         | LPUART1 (DMA, Idle detection)    |
| Toolchain    | arm-none-eabi-gcc                |
| Build system | CMake + Ninja                    |

---

## Author

**İslam Köşker** — [@islamkosker](https://github.com/islamkosker)  
Copyright © 2026 İslam Köşker.

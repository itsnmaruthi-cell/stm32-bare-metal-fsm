# STM32 Bare-Metal Button-Triggered FSM

A finite state machine on an STM32 Nucleo board, written entirely at the register level — no HAL, no CubeMX-generated peripheral init. Pressing the on-board user button (PC13) steps through four states, and the current state name is printed over UART on every transition.

## What it does

- States: `IDLE → RUNNING → ERROR → STOP → IDLE → ...`
- Each press of the user button (PC13) advances to the next state
- On every transition, the new state's name is sent over USART2 (9600 baud), one line per transition, so you can watch it live in a serial terminal

## Why bare-metal

This was written by going straight to the reference manual (RM0368) and configuring peripherals through direct register writes — RCC clock enables, GPIO MODER/AFRL, EXTI, NVIC, and USART2 — instead of HAL or CubeMX. The goal was to actually understand what each configuration bit does, not just call an init function.

## Hardware

- STM32 Nucleo board (F401/F411 family register map)
- On-board user button, PC13 (no external wiring needed — it's already pulled up on the Nucleo)
- USART2 TX/RX on PA2/PA3 (same pins used by the ST-LINK virtual COM port on most Nucleo boards, so a USB cable to your PC is enough — no external UART adapter required)

## How the pieces fit together

- **EXTI15_10_IRQHandler** — fires on a falling edge on PC13 (button press), sets a flag
- **FSM_Update()** — called every iteration of the main loop; when the flag is set, it clears it, advances `current_state`, and triggers a UART send
- **USART2_SendData()** — sends the current state's name as a string, one byte at a time, waiting for the TXE (transmit data register empty) flag before every byte

UART sending here is blocking/polled rather than interrupt-driven — deliberate for a project this size, since nothing else needs to run while a short string goes out.

## Building / Flashing

Import into STM32CubeIDE as an existing project, or open `Core/Src/main.c` directly if you're managing the build yourself. Flash to the Nucleo board and open a serial terminal at 9600 baud, 8N1 on the ST-LINK VCP port.

## Watching it work

Open a serial terminal (PuTTY, Tera Term, etc.) on the Nucleo's VCP port at 9600 baud. Each button press should print the new state on its own line:

```
STATE_RUNNING
STATE_ERROR
STATE_STOP
STATE_IDLE
```
<img width="823" height="517" alt="image" src="https://github.com/user-attachments/assets/1b56edf5-3f96-40da-b3cb-57be6e3f09ad" />


## Notes / things learned building this

- Register addresses were pulled directly from RM0368 rather than CMSIS device headers, to force actually reading the memory map
- Early versions had the classic `=` vs `==` bug, an un-dereferenced EXTI pending-register check, and UART bytes being written faster than they could physically shift out — all fixed by checking TXE before every single byte, not once per string
- `\r\n` line endings are required for the terminal to actually start a new line per state, rather than overwriting the same line each time
- Completed code with guidance.

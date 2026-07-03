# CAN bus data logger & Task scheduler

ESP32 firmware project demonstrating FreeRTOS task scheduling, CAN frame assembly and inter-task communication using queues

This project was created to gain practical knowledge on CAN bus basics. Were used such concepts as FreeRTOS, ISR, Semaphores, Queues, Ring buffers etc.

---

## Architecture overview

-> Simulator task sends bytes using Serial2.write (TX -> RX)

-> ISR fires, drains buffer into ring buffer

-> Signals semaphore

-> Reader task wakes up and assembles frames using switch-case structure, which saves data to it's designated variables

-> Reader task sends frame to queue and resets variables

-> Logger task prints out information about frame

-> Stats task prints out overall information including amount of received frames, dropped frames, overall queue depth and overrun.

---

## Components
| Component | Description |
|---|---|
|myISR | Interrupt Service Routine, drains Serial2 buffer into ring buffer, signals semaphore to wake reader task |
|ring_buffer | lock-free single-producer single-consumer byte buffer. Written by ISR, read by reader task |
|vReaderTask | main reader task, used for processing CAN frame step by step and saving data from them to variables |
|vLoggerTask | used for logging, outputs time and contents of a frame such as ID, DLC and Data |
|vSimulatorTask | simulates creation of data frame and sends it to Serial2 |
|vStatsTask | displays heath of the system: how many frames were received, dropped, total queue depth and overrun |
|setup | regular setup function, creates tasks pinned to core, starts serial and serial2, sets memory |

---

## Hardware

Microcontroller ESP32, it's GPIO 16 and 17 were connected to each other to simulate UART data exchange.

---

## How to build and flash

1. Create an empty project with ESP32 selected.

2. Upload or create main.cpp, can_frame.h, ring_buffer.cpp, ring_buffer.h

3. Make sure platformio.ini contains framework = arduino and board = esp32dev

4. Connect GPIO 16 and 17 on your ESP32 with a wire

5. Connect ESP32 to your device

6. Press PlatformIO: Build, an arrow facing right in the bottom of your screen.

---

## Performance results

| Interval | Frames/sec | Received | Dropped |
|---|---|---|---|
| 500ms | ~2 | All | 0 |
| 10ms | ~100 | All | 0 |
| 1ms | ~1000 | ~5/s | ~500/s |

---

## Debugging

ESP32 Arduino's Serial2.onReceive() isn't a true hardware ISR, but a software callback

Initial implementation read only one byte per callback causing hardware buffer to fill up, which caused it to stop working after a few frames

**Fix**: draining all available bytes inside the callback clears the hardware buffer completely, preventing stalls

*P.S. On direct UART interrupt registers (Like STM32 board has) this issue doesn't exist - the ISR fires per byte at the hardware level*

---

## What I learned

During my work on this projects I learned what CAN bus consists of and how to build code for one 

Before I only knew theory, now I'm capable of building one myself :)

I learned how to structure firmware around independent tasks that communicates through structures like semaphores and queues, rather than using single loop for everything

Also this was my first time working on interrupts - my previous projects didn't require them. I encountered issue with my original implementation, came up with temporary solution and after some debugging, fixed ISR and came back to original, cleaner implementation

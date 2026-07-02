#include <Arduino.h>
#include <string.h>
#include "ring_buffer.h"
#include "can_frame.h"

SemaphoreHandle_t xSemaphore; // Declares semaphore
QueueHandle_t xQueue; // Holds reference to a queue. Declares a variable
Ring_Buffer rb;

volatile uint32_t frames_received = 0;
volatile uint32_t frames_dropped = 0;

void myISR(){
    uint8_t byte;
    byte = Serial2.read();
    rb_write(&rb, byte);

    BaseType_t higherPriorityWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemaphore, &higherPriorityWoken);
    portYIELD_FROM_ISR(higherPriorityWoken);
}

void vReaderTask (void *pvParameters){ // reader task
    FrameState state = WAIT_SOF;
    CAN_frame_t frame;
    uint8_t byte_count = 0;
    uint8_t byte;

    vTaskDelay(pdMS_TO_TICKS(100));
    
    while (1){
        Serial2.onReceive(myISR);
        xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(50));
        while (rb_available(&rb) != 0) {
            rb_read(&rb, &byte);
            switch(state){
                case WAIT_SOF: // Waiting for 0xAA
                    if (byte == 0xAA){
                        state = READ_ID_H;
                    }
                    break;
            
                case READ_ID_H: // Read higher byte of ID
                    frame.id = byte << 8;
                    state = READ_ID_L;
                    break;
        
                case READ_ID_L: // Read lower byte of ID
                    frame.id |= byte;
                    state = READ_DLC;
                    break;

                case READ_DLC: // data length
                    frame.dlc = byte;
                    state = READ_DATA;
                    break;
            
                case READ_DATA: // read data 
                    frame.data[byte_count] = byte;
                    byte_count++;
                    if (byte_count == frame.dlc) {
                        state = READ_CRC;
                    }
                    break;

                case READ_CRC: // send frame to queue and reset variables
                    state = WAIT_SOF;
                    byte_count = 0;
                    if (xQueueSend(xQueue, &frame, 0) == pdTRUE) {
                        frames_received++;
                    } 
                    else {
                        frames_dropped++;
                    }
                    break;
            }
        }
    }
}

void vLoggerTask(void *pvParameters) {
    CAN_frame_t frame;
    while(1){
        xQueueReceive(xQueue, &frame, portMAX_DELAY);
        Serial.print("\n>> Frame\n");
        Serial.printf("[%lu ms] | ID:0x%03X | DLC:%d \n", xTaskGetTickCount(), frame.id, frame.dlc);
        Serial.printf("DATA: ");
        for (int i = 0; i < frame.dlc; i++) {
            Serial.printf("%02X ", frame.data[i]);
        }
        Serial.printf("\n");
    }
}

void vSimulatorTask(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(200));
    while(1){
        uint8_t frame[] = {0xAA, 0x07, 0xE8, 0x03, 0xAD, 0xDE, 0xEB, 0x00};
        Serial2.write(frame, sizeof(frame)); 
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vStatsTask(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(200));
    while(1){
        Serial.printf("\n>> STATS\nReceived: %lu\nDropped: %lu\nQueue depth: %u\n", frames_received, frames_dropped, uxQueueMessagesWaiting(xQueue));
        Serial.printf("Overrun: %d\n", rb.overrun);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void setup() {
    Serial.begin(115200); // Baud - symbols per second
    

    Serial2.begin(115200); // 
    Serial2.onReceive(myISR); // call this function when something happens

    xSemaphore = xSemaphoreCreateBinary();

    memset(&rb, 0, sizeof(Ring_Buffer));

    xQueue = xQueueCreate(5, sizeof(CAN_frame_t));
    // 5 - how many iterms the queue can hold at once
    // sizeof - size of each item in bytes

    xTaskCreatePinnedToCore(vReaderTask, "ReaderTask", 5000, NULL, 3, NULL, 1);
    // Creates and starts task
    // Allocates the stack memory (2048)
    // Registers the function as task with the priority set (3)
    // Starts scheduling (will run as soon as scheduler gets control)
    // NULL - parameter to pass in and where to store the task handle
    // 1 - which core to run on

    xTaskCreatePinnedToCore(vLoggerTask, "LoggerTask", 5000, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(vSimulatorTask, "SimulatorTask", 5000, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(vStatsTask, "StatsTask", 5000, NULL, 1, NULL, 1);
}

void loop() {
    // empty
}
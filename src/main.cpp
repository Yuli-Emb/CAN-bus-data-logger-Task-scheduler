#include <Arduino.h>
#include "ring_buffer.h"
#include <string.h>

SemaphoreHandle_t xSemaphore; // Declares semaphore
QueueHandle_t xQueue; // Holds reference to a queue. Declares a variable
Ring_Buffer rb;

void vReaderTask (void *pvParameters){ // First task
    vTaskDelay(pdMS_TO_TICKS(100));
    for (int i = 0; i < 5; i++) {
        Serial2.write('A');
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    while (1){
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(10));
        uint8_t byte;
        while(rb_available(&rb) != 0){
            rb_read(&rb, &byte);
            Serial.printf("0x%02X\n", byte);
        }
    }
}

void myISR(){
    uint8_t byte;
    byte = Serial2.read();
    rb_write(&rb, byte);

    BaseType_t higherPriorityWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemaphore, &higherPriorityWoken);

    portYIELD_FROM_ISR(higherPriorityWoken);
}

void setup() {
    Serial.begin(115200); // Baud - symbols per second
    

    Serial2.begin(115200); // 
    Serial2.onReceive(myISR); // call this function when something happens

    xSemaphore = xSemaphoreCreateBinary();

    memset(&rb, 0, sizeof(Ring_Buffer));

    xQueue = xQueueCreate(5, sizeof(int));
    // 5 - how many iterms the queue can hold at once
    // sizeof - size of each item in bytes

    xTaskCreatePinnedToCore(vReaderTask, "ReaderTask", 2048, NULL, 3, NULL, 1);
    // Creates and starts task
    // Allocates the stack memory (2048)
    // Registers the function as task with the priority set (3)
    // Starts scheduling (will run as soon as scheduler gets control)
    // NULL - parameter to pass in and where to store the task handle
    // 1 - which core to run on
}

void loop() {
    // empty
}
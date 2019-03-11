#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "drivers/rgb.h"
#include "drivers/buttons.h"
#include "utils/uartstdio.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "servo_task.h"
#include "servo.h"

//*****************************************************************************
//
// The stack size for the LED toggle task.
//
//*****************************************************************************
#define SERVOTASKSTACKSIZE        128         // Stack size in words

//*****************************************************************************
//
// The item size and queue size for the LED message queue.
//
//*****************************************************************************
#define SERVO_ITEM_SIZE           sizeof(uint8_t)
#define SERVO_QUEUE_SIZE          5

//*****************************************************************************
//
// Default LED toggle delay value. LED toggling frequency is twice this number.
//
//*****************************************************************************
#define SERVO_TOGGLE_DELAY        1000

//*****************************************************************************
//
// The queue that holds messages sent to the LED task.
//
//*****************************************************************************
xQueueHandle g_pServoQueue;

extern xSemaphoreHandle g_pUARTSemaphore;

typedef enum{
  WAIT,
  SLOTNUM,
  DISPENSING
} SERVOSTATE;

SERVOSTATE servo_state;

static void ServoTask(void *pvParameters){
    portTickType ui32WakeTime;
    uint32_t ui32ServoDelay;
    uint8_t i8Message;
    
    ui32WakeTime = xTaskGetTickCount();
    
    while(1){
      switch(servo_state){
        case WAIT: 
          if(xQueueReceive(g_pServoQueue, &i8Message, 0) == pdPASS){
              if(i8Message == DISPENSE){
                  
                  servo_state = SLOTNUM;
                
                  xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                  UARTprintf("Waiting for slot number");
                  xSemaphoreGive(g_pUARTSemaphore);
              }
              
          }
          break;
        case SLOTNUM:
          if(xQueueReceive(g_pServoQueue, &i8Message, 0) == pdPASS){
              uint8_t slotNum = i8Message;
              servo_state = DISPENSING;
              servo_write(slotNum, SERVO_MAX);
              
              ui32ServoDelay *= 2;
              if(ui32ServoDelay > 1000)
              {
                  ui32ServoDelay = SERVO_TOGGLE_DELAY / 2;
              }
              
              xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
              UARTprintf("Dispensing slot %d", slotNum);
              xSemaphoreGive(g_pUARTSemaphore);
              
              vTaskDelayUntil(&ui32WakeTime, ui32ServoDelay / portTICK_RATE_MS);
              
              servo_write(slotNum, SERVO_MIN);
          }
          break;
        case DISPENSING: 
          servo_state = WAIT;
          
          xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
          UARTprintf("Finish dispensing pill");
          xSemaphoreGive(g_pUARTSemaphore);
          
          i8Message = SERVOREADY;
          
          if(xQueueSend(g_pUARTQueue, &ui8Message, portMAX_DELAY) != pdPASS){
              //
              // Error. The queue should never be full. If so print the
              // error message on UART and wait for ever.
              //
              UARTprintf("\nQueue full. This should never happen.\n");
              while(1)
              {
              }
          }
          break;
        default: 
          servo_state = WAIT;
          break;
      }
    }
}
uint32_t ServoTaskInit(void){
  
    servo_state = WAIT;
    
    //
    // Create a queue for sending messages to the LED task.
    //
    g_pServoQueue = xQueueCreate(SERVO_QUEUE_SIZE, SERVO_ITEM_SIZE);

    //
    // Create the servo task.
    //
    if(xTaskCreate(ServoTask, (const portCHAR *)"Servo", SERVOTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_SERVO_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}

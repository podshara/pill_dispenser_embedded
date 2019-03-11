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
#include "UART_task.h"

//*****************************************************************************
//
// The stack size for the LED toggle task.
//
//*****************************************************************************
#define UARTTASKSTACKSIZE        128         // Stack size in words

//*****************************************************************************
//
// The item size and queue size for the LED message queue.
//
//*****************************************************************************
#define UART_ITEM_SIZE           sizeof(uint8_t)
#define UART_QUEUE_SIZE          5

//*****************************************************************************
//
// Default LED toggle delay value. LED toggling frequency is twice this number.
//
//*****************************************************************************
#define UART_TOGGLE_DELAY        1000

//*****************************************************************************
//
// The queue that holds messages sent to the LED task.
//
//*****************************************************************************
xQueueHandle g_pUARTQueue;

extern xSemaphoreHandle g_pUARTSemaphore;

static void UARTTask(void *pvParameters){
    portTickType ui32WakeTime;
    uint8_t i8Message;
    
    ui32WakeTime = xTaskGetTickCount();
    
    while(1){
      if(xQueueReceive(g_pUARTQueue, &i8Message, 0) == pdPASS){
          if(i8Message == UPDATEPILL){
              //UARTReceive();
              
              xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
              UARTprintf("Update pill");
              xSemaphoreGive(g_pUARTSemaphore);
              
              i8Message = UARTREADY;
              
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
          }
          
      }
    }
}
uint32_t UARTTaskInit(void){
    
    //
    // Create a queue for sending messages to the LED task.
    //
    g_pUARTQueue = xQueueCreate(UART_QUEUE_SIZE, UART_ITEM_SIZE);

    //
    // Create the LCD task.
    //
    if(xTaskCreate(UARTTask, (const portCHAR *)"UART", UARTTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_UART_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}

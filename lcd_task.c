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
#include "lcd_task.h"
#include "LCD_display.h"
#include "pillslot.h"

//*****************************************************************************
//
// The stack size for the LED toggle task.
//
//*****************************************************************************
#define LCDTASKSTACKSIZE        128         // Stack size in words

//*****************************************************************************
//
// The item size and queue size for the LED message queue.
//
//*****************************************************************************
#define LCD_ITEM_SIZE           sizeof(uint8_t)
#define LCD_QUEUE_SIZE          5

//*****************************************************************************
//
// Default LED toggle delay value. LED toggling frequency is twice this number.
//
//*****************************************************************************
#define LCD_TOGGLE_DELAY        250

//*****************************************************************************
//
// The queue that holds messages sent to the LED task.
//
//*****************************************************************************
xQueueHandle g_pLCDQueue;

extern xSemaphoreHandle g_pUARTSemaphore;

static void LCDTask(void *pvParameters){
    portTickType ui32WakeTime;
    uint8_t x, y, touch;
    uint8_t i8Message;
    
    while(1){
      
        readCoor(&x, &y, &touch);
        if(touch){
          switchPage(x, y);
        }
    
        if(xQueueReceive(g_pLCDQueue, &i8Message, 0) == pdPASS){
            if(i8Message == UPDATELCD){
                
                Draw_Next();
                
                xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
                UARTprintf("Updating screen");
                xSemaphoreGive(g_pUARTSemaphore);
                
                i8Message = LCDREADY;
                
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
uint32_t LCDTaskInit(void){
  
    page = NEXT;
    Draw_Next();
    
    //
    // Create a queue for sending messages to the LED task.
    //
    g_pLCDQueue = xQueueCreate(LCD_QUEUE_SIZE, LCD_ITEM_SIZE);

    //
    // Create the LCD task.
    //
    if(xTaskCreate(LCDTask, (const portCHAR *)"LCD", LCDTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_LCD_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}

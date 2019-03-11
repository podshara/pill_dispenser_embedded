#include "inc/tm4c123gh6pm.h"
#include "servo.h"
#include "uart.h"
#include "LCD_display.h"
#include "SSD2119.h"


int main() {
  LCD_Init();
  servo_init();
  uart1_Init();
}
#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "servo.h"
#include "uart_pd.h"
#include "LCD_display.h"
#include "SSD2119.h"
#include "pillslot.h"

void dispense(int slotNum){
  dispenseSlot(slotNum);
}

void updateName(int slotNum) {
  char pillName[32];
  int i = 0;
  int input = 0;
  while(input != '\n'){
    input = uart1_inchar();
    if(input == '\n'){
      pillName[i] = '\0'; // end char
    } else {
      pillName[i] = input;
    }
    i++;
  }
  setName(slotNum, pillName);
  updateNameDisplay(slotNum);
}

void updateCount(int slotNum) {
  int amount = (uart1_inchar() - '0') * 10;
  amount += uart1_inchar() - '0';
  setAmount(slotNum, amount);
  updateCountDisplay(slotNum);
}

int main() {
  pll_Init();
  LCD_Init();
  Touch_Init();
  servo_Init();
  uart1_Init();
  
  Draw_Slots();
  uart1_outchar('R');
  while(1){
    
    int x, y, touch;
    readCoor(&x, &y, &touch);
    if(touch){
      switchPage(x, y);
    }
    
    char c = uart1_inchar_non_block();    
    if(c != 0){
      //LCD_Goto(0, 3);
      //printf("%c", c);
      if (c >= '1' && c <= '4') {
        dispense(c - '1');
      } else if(c == 'N'){
        updateName(uart1_inchar() - '1');
      } else if(c == 'C'){
        updateCount(uart1_inchar() - '1');
      } else if(c == 'D') {
        int slot = uart1_inchar() - '1';
        removeSlot(slot);
        printPill(slot);
      }
      /*else if(c == 'T'){
        int slotNum = uart1_inchar() - '1';
        int timeSize = (uart1_inchar() - '1') * 10;
        timeSize = uart1_inchar() - '1';
        
        int time[MAX_TIMESET];
        for(int i = 0; i < timeSize; i++){
          for(int j = 0; j < 4; j++) {
            time[i] = time[i] * 10 + (uart1_inchar() - '1');
          }
        }
        
        setTime(slotNum, time, timeSize);
      }*/
    }
  }
}
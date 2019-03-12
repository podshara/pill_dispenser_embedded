#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "servo.h"
#include "uart_pd.h"
#include "LCD_display.h"
#include "SSD2119.h"
#include "pillslot.h"

void updatePill() {
  char input;
  int slotNum;
  for(int slot = 0; slot < MAX_SLOT; slot++){
    int amount, timeSize;
    int time[MAX_TIMESET];
    char pillName[32];

    slotNum = uart1_inchar() - '0';
    
    int i = 0;
    while(input != '\n'){
      input = uart1_inchar();
      if(input == '\n'){
        pillName[i] = '\0'; // end char
      } else {
        pillName[i] = input;
      }
      i++;
    }

    amount = (uart1_inchar() - '0') * 10;
    amount += uart1_inchar() - '0';
    
    timeSize = (uart1_inchar() - '0') * 10;
    timeSize = uart1_inchar() - '0';
    
    for(int i = 0; i < timeSize; i++){
      for(int j = 0; j < 4; j++) {
        time[i] = time[i] * 10 + (uart1_inchar() - '0');
      }
    }
    
    setSlot(slotNum, pillName, amount);
    for(int t = 0; t < timeSize; t++){
      addTime(slotNum, time[t]);
    }
    
  }

  Draw_Next();
}

void dispense(char slot){
  int slotNum =  slot - '1';
  dispenseSlot(slotNum);
  LCD_Goto(0, 2);
  printf("pushed");
}

int main() {
  pll_Init();
  LCD_Init();
  Touch_Init();
  servo_Init();
  uart1_Init();
  
  Draw_Slots();
  while(1){
    
    int x, y, touch;
    readCoor(&x, &y, &touch);
    if(touch){
      switchPage(x, y);
    }
    
    char c = uart1_inchar_non_block();    
    if(c != 0){
      LCD_Goto(0, 3);
      printf("%c", c);
      if (c >= '1' && c <= '4') {
        dispense(c);
      } else if(c == 'N'){
        int slotNum = uart1_inchar() - '1';
        
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
        
        //setSlot(slotNum, pillName, 0);
        setName(slotNum, pillName);
        
        //Draw_Slots();
        
      } else if(c == 'C'){
        int slotNum = uart1_inchar() - '1';
        
        int amount = (uart1_inchar() - '0') * 10;
        amount += uart1_inchar() - '0';
        
        
        setAmount(slotNum, amount);
        
        Draw_Slots();
        LCD_Goto(0, 5);
        printf("%d", slotNum);
        LCD_Goto(0,7);
        printf("%d", amount);
        
      }
      
      
      else if(c == 'T'){
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
        
        Draw_Slots();
      }
    } 
  }
}
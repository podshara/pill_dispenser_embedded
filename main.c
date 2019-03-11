#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "servo.h"
#include "uart_pd.h"
#include "PLL.h"
#include "LCD_display.h"
#include "SSD2119.h"
#include "pillslot.h"

#define BUFFER          5

void Timer0_Init(){
  SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0;    //use timer 0
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN;              //disable timer
  TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;        //select 32 bit timer config
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_1_SHOT;       //countdown + one-shot mode
  TIMER0_TAILR_R = F_CPU;                       //16MHz
}

void StartTimer0(){
  TIMER0_CTL_R |= TIMER_CTL_TAEN;
}

void DisableTimer0(){
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN;
}

void ResetTimer0(){
  TIMER0_ICR_R |= TIMER_ICR_TATOCINT;
}

void setTime0(int time){
  TIMER0_TAILR_R = time;
}

int Timeout0(){
  return TIMER0_RIS_R & TIMER_RIS_TATORIS;
}

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

void dispense(){
  int slotNum =  uart1_inchar() - '0';
  dispensePill(slotNum);
      
  servo_write(slotNum, SERVO_MIN);
  setTime0(F_CPU);
  StartTimer0();
  
  while(!Timeout0()){}
  ResetTimer0();
  servo_write(slotNum, SERVO_MAX);
}

int main() {
  PLL_Init();
  LCD_Init();
  Touch_Init();
  servo_Init();
  uart1_Init();
  Timer0_Init();
  
  Draw_Next();
  while(1){
    uart1_outchar('k');
    
    int x, y, touch;
    readCoor(&x, &y, &touch);
    if(touch){
      switchPage(x, y);
    }
    
    char c = uart1_inchar_non_block();    
    LCD_Goto(0, 2);
    printf("%c", c);
    if(c != 0){
      LCD_Goto(0, 2);
      printf("kuy");
      servo_write(0, 180);
    }
    switch(c){
      case 'R':
        updatePill();
        break;
      case 'D':
        dispense();
        break;
      default: break;
    }
  }
}
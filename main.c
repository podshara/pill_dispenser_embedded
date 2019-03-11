#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "servo.h"
#include "uart_pd.h"
#include "PLL.h"
#include "LCD_display.h"
#include "SSD2119.h"
#include "pillslot.h"

void UART_Init(void){
  SYSCTL_RCGCUART_R |= 0x01;            // activate UART0
  SYSCTL_RCGCGPIO_R |= 0x04;            // activate port C
  UART0_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART0_IBRD_R = 8;                     // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.680)
  UART0_FBRD_R = 44;                    // FBRD = round(0.5104 * 64 ) = 33
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0_CTL_R |= UART_CTL_UARTEN;       // enable UART
  GPIO_PORTC_AFSEL_R |= 0x30;           // enable alt funct on PA1-0
  GPIO_PORTC_DEN_R |= 0x30;             // enable digital I/O on PA1-0
                                        // configure PA1-0 as UART
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000;
  GPIO_PORTC_AMSEL_R &= ~0x30;          // disable analog functionality on PA
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
char UART_InChar(void){
  while((UART0_FR_R&UART_FR_RXFE) != 0);
  return((char)(UART0_DR_R&0xFF));
}
//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(char data){
  while((UART0_FR_R&UART_FR_TXFF) != 0);
  UART0_DR_R = data;
}

#define BUFFER          5

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
  dispenseSlot(slotNum);
  LCD_Goto(0, 2);
  printf("pushed");
}

int main() {
  //PLL_Init();
  //LCD_Init();
  //Touch_Init();
  //servo_Init();
  //uart1_Init();
  UART_Init();
  
  Draw_Next();
  while(1){
    //uart1_outchar('k');
    UART_OutChar('a');
    
    /*int x, y, touch;
    readCoor(&x, &y, &touch);
    if(touch){
      switchPage(x, y);
      //dispenseSlot(0);
    }
    
    char c = uart1_inchar_non_block();    
    LCD_Goto(0, 3);
    printf("%c", c);
    if(c != 0){
      LCD_Goto(0, 2);
      printf("kuy");
      
    }
    switch(c){
      //case 'R':
        //updatePill();
        //break;
      case 'D':
        dispense();
        LCD_Goto(0,2);
        printf("      ");
        break;
      default: break;
    }*/
  }
}
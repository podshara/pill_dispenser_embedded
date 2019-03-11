#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include "uart_pd.h"

#define DIVIDER ((clock_rate * 1000000.0) / (16.0 * baud_rate))

int clock_rate = 16;           // MHz
int baud_rate = 115200;

int get_ibrd() {
  return (int) DIVIDER;
}

int get_fbrd() {
  double divider = DIVIDER;
  return (int)((divider - (int) divider) * 64.0 + 0.5);
}


void uart1_Init() {
  SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1;                                      //use UART 1
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;                                      //enable clock to the port C
  GPIO_PORTC_AFSEL_R |= 0x30;                                                   //enable PC4-5 alt hardware function
  GPIO_PORTC_DEN_R |= 0x30;                                                     //enable PC4-5
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)|0x00220000;                //set port control of PC4-5 to U1R/T
  UART1_CTL_R &= ~UART_CTL_UARTEN;                                              //disable UART
  UART1_IBRD_R = (UART1_IBRD_R&~UART_IBRD_DIVINT_M)|get_ibrd();                 //set divider according to 
  UART1_FBRD_R = (UART1_FBRD_R&~UART_FBRD_DIVFRAC_M)|get_fbrd();                //clock_rate and baud_rate
  UART1_LCRH_R = (UART1_LCRH_R&~UART_LCRH_WLEN_M)|UART_LCRH_WLEN_8;             //set word size to 8 bits
  //UART1_CC_R = (UART1_CC_R&~UART_CC_CS_M)|UART_CC_CS_SYSCLK;                    //system clock as UART baud source
  UART1_CC_R = 0;
  UART1_CTL_R |= UART_CTL_RXE + UART_CTL_TXE;                                   //enable UART transmit and revieve
  UART1_CTL_R |= UART_CTL_UARTEN;                                               //enable UART
}

void uart1_outchar(char c) {
  while(UART1_FR_R& UART_FR_TXFF){}
  UART1_DR_R = c;
}

void uart1_outstring(char* s) {
  while(*s){
    uart1_outchar(*(s++));
  }
}

char uart1_inchar() {
  while(UART1_FR_R&UART_FR_RXFE){}
  return((char)(UART1_DR_R&UART_DR_DATA_M));
}

char uart1_inchar_non_block() {
  if(!(UART1_FR_R&UART_FR_RXFE)){
    return((char)(UART1_DR_R&UART_DR_DATA_M));
  } else{
    return 0;
  }
}
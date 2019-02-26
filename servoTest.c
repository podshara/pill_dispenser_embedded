#include <stdint.h>
#include <tm4c123gh6pm.h>
#include "servo.h"

//----Timer----
void Timer0_Init(){
  SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0;  //use timer 0
  TIMER0_CTL_R = 0x0;                        //disable timer
  TIMER0_CFG_R = 0x00000000;                 //select 32 bit timer config
  TIMER0_TAMR_R = 0x00002;                   //countdown + periodic mode
  TIMER0_TAILR_R = 16000000;                 //16MHz
  TIMER0_CTL_R |= 0x1;                       //start timer
  
  //TIMER0_IMR_R = 0x1;                        //interrupt mask
  //NVIC_EN0_R = 0x80000;                      //enable interrupt 19
}

void StartTimer0(){
  TIMER0_CTL_R |= 0x1;
}

void DisableTimer0(){
  TIMER0_CTL_R &= ~0x1;
}

void ResetTimer0(){
  TIMER0_ICR_R |= 0x1;
}

int Timeout0(){
  return TIMER0_RIS_R & 0x1;
}

//----PLL----
void pll_Init(){
  SYSCTL_RCC2_R |= (0x1 << 31);                 //use RCC2
  SYSCTL_RCC2_R |= (0x1 << 11);                 //set bypass before PLL config
  SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_XTAL_M) | (0x15 << 6);//Xtal 16MHz
  SYSCTL_RCC2_R &= ~(0x7 << 4);                 //select main osc
  SYSCTL_RCC2_R &= ~(0x1 << 13);                //activate PLL
  SYSCTL_RCC2_R |= (0x1 << 30);                 //DIV400 -> 400 MHz
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~(SYSCTL_RCC2_SYSDIV2_M + 
                SYSCTL_RCC2_SYSDIV2LSB)) | (24 << 22); //set SysDiv -> 16 MHz
  SYSCTL_RCC_R |= (0x1 << 22);                  //use SysDiv
  
  while(!(SYSCTL_RIS_R & (0x1 << 6))){}         //wait PLL to lock
  SYSCTL_RCC2_R &= ~(0x1 << 11);                //clear bypass
}

//----PortF----
void PortF_Init(){
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
  
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
  GPIO_PORTF_DATA_R = 0;
}

int sw1_input(){
  return GPIO_PORTF_DATA_R & 0x10;
}

void setLED(int color){
  GPIO_PORTF_DATA_R &= ~0xE;
  GPIO_PORTF_DATA_R |= (1 << color);
}

int turn = 0;

void foo(){
  if(turn){
    setLED(1);
    servo_write(0, 45);
    
  } else {
    setLED(0);
    servo_write(0, 140);
    
  }
  turn = !turn;
}

int main(){
  Timer0_Init();
  PortF_Init();
  pll_Init();
  servo_Init();
  
  setLED(1);
  int sw1 = 0;
  while(1){
    if(sw1_input()){
      if(!sw1){
        sw1 = 1;
        foo();
      }
    } else {
      sw1 = 0;
    }
  }
}
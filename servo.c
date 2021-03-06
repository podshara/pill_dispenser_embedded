#include <stdint.h>

#include "inc/tm4c123gh6pm.h"
#include "servo.h"
#include "uart.h"

volatile uint32_t ticksPerMicrosecond;
uint32_t curServo;
uint32_t servoPulse[SERVO_NUM];
uint32_t enable[SERVO_NUM];
uint32_t remainderPulse;

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

void timer2A_Init() {
  SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R2;    //use timer2A
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN;              //disable timer
  TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;        //use 32 bit configuration
  TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;       //use period-timer
  TIMER2_TAILR_R = (ticksPerMicrosecond * REFRESH_INTERVAL);
  TIMER2_ICR_R |= TIMER_ICR_TATOCINT;  
  TIMER2_IMR_R |= TIMER_IMR_TATOIM;             //enable interrupt
  //NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x40000000;  //priority 2
  NVIC_EN0_R |= (1<<23);                        //enable interrupt 23 in NVIC 
  TIMER2_CTL_R |= TIMER_CTL_TAEN;               //enable timer
}

void portD_Init() { volatile unsigned long delay;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD; 
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY;    //unlock PD
  GPIO_PORTD_CR_R |= 0x1F;              //allow changes to PD0-5
  GPIO_PORTD_AMSEL_R &= ~0x1F;          //disable analog on PD0-5
  GPIO_PORTD_PCTL_R &= ~0x1F;           //PCTL GPIO on PD0-5
  GPIO_PORTD_DIR_R |= 0x1F;             //PD0-5 output
  GPIO_PORTD_AFSEL_R &= ~0x1F;          //disable alt funct on PD0-5
  GPIO_PORTD_DEN_R |= 0x1F;             //enable PD0-5
  GPIO_PORTD_DATA_R = 0x00;             //clear data
}

void calcuatePeriod() {
  uint32_t servoPeriodSum = 0;
  for (int i = 0; i < SERVO_NUM; i++) {
    servoPeriodSum += servoPulse[i];
  }
  remainderPulse = REFRESH_INTERVAL - servoPeriodSum;
}

void servo_Init() {
  Timer0_Init();
  
  ticksPerMicrosecond = F_CPU / 1000000;
  portD_Init();
  
  for (int i = 0; i < SERVO_NUM; i++) {
    servoPulse[i] = DEFAULT_SERVO_PULSE_WIDTH;
    enable[i] = 1;
  }
  calcuatePeriod();
  curServo = 0;
  
  timer2A_Init();
  
  for (int i = 0; i < SERVO_NUM; i++) {
    servo_write(i, SERVO_MIN);
  }
}

void servo_write(uint32_t index, uint32_t val) {
  if (val < MIN_SERVO_PULSE_WIDTH) {
    if(val < SERVO_MIN) {val = SERVO_MIN;}
    if(val > SERVO_MAX) {val = SERVO_MAX;}
    val = val * (MAX_SERVO_PULSE_WIDTH - MIN_SERVO_PULSE_WIDTH) / 180 + MIN_SERVO_PULSE_WIDTH;
    servoPulse[index] = val;
  }
}

void dispenseSlot(int slotNum){
  /*if(slotNum == 4){
    slotNum = 0;
  }*/
  servo_write(slotNum, SERVO_MAX);
  setTime0(F_CPU);
  StartTimer0();
  
  while(!Timeout0()){}
  ResetTimer0();
  servo_write(slotNum, SERVO_MIN);
  setTime0(F_CPU);
  StartTimer0();
  
  while(!Timeout0()){}
  ResetTimer0();
}

void Timer2A_Handler() {
  if (curServo < SERVO_NUM) {
    TIMER2_TAILR_R = ticksPerMicrosecond * servoPulse[curServo];
  } else {
    TIMER2_TAILR_R = ticksPerMicrosecond * remainderPulse;  
  }
  if (curServo > 0 && enable[curServo - 1]) {
    GPIO_PORTD_DATA_R &= ~(1<<(curServo - 1));
  }
  
  if (curServo < SERVO_NUM) {
    if(enable[curServo]) {
      GPIO_PORTD_DATA_R |= (1<<curServo);
    }
    curServo++;
  } else {
    curServo = 0;
  }
  TIMER2_ICR_R |= TIMER_ICR_TATOCINT;
}

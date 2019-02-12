#include <stdint.h>
#include <tm4c123gh6pm.h>

#include "servo.h"

volatile uint32_t ticksPerMicrosecond;
uint32_t curServo;
uint32_t servoPulse[SERVO_NUM];
uint32_t enable[SERVO_NUM];

void timer2A_Init() {
  SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R2;    //use timer2A
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN;              //disable timer
  TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;        //use 32 bit configuration
  TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;       //use 1 shot-timer
  TIMER2_TAILR_R = ticksPerMicrosecond * REFRESH_INTERVAL;    
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;             //enable interrupt
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

void servo_Init() {
  timer2A_Init();
  portD_Init();
  
  for (int i = 0; i < SERVO_NUM; i++) {
    servoPulse[i] = DEFAULT_SERVO_PULSE_WIDTH;
    enable[i] = 1;
  }
  ticksPerMicrosecond = F_CPU / 1000000;
}

void calcuatePeriod() {
  uint32_t servoPeriodSum = 0;
  for (int i = 0; i < SERVO_NUM; i++) {
    servoPeriodSum += servoPulse[i];
  }
  remainderPulse = REFRESH_INTERVAL - servoPeriodSum;
}

void servo_write(uint32_t index, uint32_t val) {
  if (val < MIN_SERVO_PULSE_WIDTH) {
    if(val < 0) {val = 0;}
    if(val > 180) {val = 180;}
    val = val * (MAX_SERVO_PULSE_WIDTH - MIN_SERVO_PULSE_WIDTH) / 180 + MIN_SERVO_PULSE_WIDTH;
    servoPulse[index] = val;
  }
}

/*temp*/
void attach(uint32_t index) {}

/*temp*/
void detach(uint32_t index) {}

/*temp*/
void Timer2A_Handler() {
  TIMER0_ICR_R |= TIMER_ICR_TATOCINT;
  if (curServo < SERVO_NUM) {
    TIMER2_TAILR_R = ticksPerMicrosecond * servoPulse[curServo];
  } else {
    TIMER2_TAILR_R = ticksPerMicrosecond * remainderPulse;  
  }
  
  if (curServo > 0 && enable[curServo]) {
    GPIO_PORTD_DATA_R ~= (1<<curServo);
  }
  
  if (curServo < SERVO_NUM) {
    if(enable[curServo]) {
      GPIO_PORTD_DATA_R |= (1<<curServo);
    }
    curServo++;
  } else {
    curServo = 0;
  }
}

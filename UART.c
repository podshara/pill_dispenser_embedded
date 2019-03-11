#include <tm4c123gh6pm.h>
#include <stdint.h>
#include <stdio.h>

#define BAUDRATE 115200

//----Timer----
void Timer0_Init(){
  SYSCTL_RCGCTIMER_R = SYSCTL_RCGCTIMER_R0;  //enable timer 0
  TIMER0_CTL_R = 0x0;                        //disable timer
  TIMER0_CFG_R = 0x00000000;                 //select 32 bit timer config
  TIMER0_TAMR_R = 0x00002;                   //countdown + periodic mode
  TIMER0_TAILR_R = 0xF42400;                //coundown 1 sec, 16M
  
  TIMER0_CTL_R |= (0x1 << 5);                //enable ADC trigger
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

void setTime0(int time){
  TIMER0_TAILR_R = time;
}

int Timeout0(){
  return TIMER0_RIS_R & 0x1;
}

//----UART----
void UART_Init(){
  SYSCTL_RCGCUART_R |= 0x2;     //use UART 1
  UART1_CTL_R &= ~0x1;          //disable UART
  UART1_IBRD_R = 8;             //baud rate 16MHz
  UART1_FBRD_R = 44;
  UART1_LCRH_R |= 0x60;         //serial param 8 bits
  UART1_LCRH_R |= (1 << 4);     //enable FIFO
  UART1_CC_R &= ~0xF;           //use system clk
  UART1_CTL_R |= 0x1;           //enable UART
  UART1_CTL_R |= (3 << 8);      //enable TxRx
}

int BusyFlag(){
  return UART1_FR_R & (1 << 3);
}

int TxFull(){
  return UART1_FR_R & (1 << 5);
}

int RxEmpty(){
  return UART1_FR_R & (1 << 4);
}

void WriteChar(char c){
  while(TxFull());
  UART1_DR_R = c;
}

void WriteString(char * data){
  while(*data){
    WriteChar(*(data++));
  }
}

int ReadData(){
  return UART1_DR_R & 0xFF;
}

//----PLL----
void PLL_Init(){
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

//----Port----
void Port_Init(){
  SYSCTL_RCGCGPIO_R |= 0x2;     //enable port B
  GPIO_PORTB_AFSEL_R |= 0x3;    //Alternate func B0,B1
  GPIO_PORTB_PCTL_R |= 0x11;     //mode select B0,B1
  GPIO_PORTB_DEN_R |= 0x3;      //digital enable
  
  //SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;
  SYSCTL_RCGCGPIO_R |= (1 << 5);    //enable port F
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

int SW1_Input(){
  return !(GPIO_PORTF_DATA_R & 0x10);
}

int SW2_Input(){
  return !(GPIO_PORTF_DATA_R & 0x1);
}

void addColor(int color){
  GPIO_PORTF_DATA_R |= (color << 1);
}

void removeColor(int color){
  GPIO_PORTF_DATA_R &= ~(color << 1);
}

void setColor(int color){
  GPIO_PORTF_DATA_R &= ~(7 << 1);
  addColor(color);
}

//----ADC----
void ADC_Init(){
  SYSCTL_RCGCADC_R |= 0x1;              //enable ADC clock module 0
  
  SYSCTL_RCGCGPIO_R |= (0x1 << 4);      //enable clock to port E
  GPIO_PORTE_AFSEL_R |= 0x8;            //set Alternate func E3
  GPIO_PORTE_DEN_R &= ~0x8;             //set analog input E3
  GPIO_PORTE_AMSEL_R |= 0x8;            //disable analog isolation E3
  
  ADC0_ACTSS_R &= ~0x8;                 //disable sample sequencer 3
  ADC0_EMUX_R |= (0x5 << 12);           //trigger by timer
  
  ADC0_SSMUX3_R &= ~0xF;                //config input src Ain0
  ADC0_SSCTL3_R |= 0x2;                 //set END bit
  ADC0_SSCTL3_R |= 0x4;                 //interrupt enable
  ADC0_SSCTL3_R |= 0x8;                 //Temp sensor
  ADC0_IM_R |= 0x8;                     //send RIS
  ADC0_ACTSS_R |= 0x8;                  //enable sample sequence 3
  
  ADC0_PSSI_R |= 0x8;                   //start conversion
  
  NVIC_EN0_R |= (0x1 << 17);            //enable interrupt 17
}

int Read_ADC0(){
  return ADC0_SSFIFO3_R;
}

void ClearADC0_Input(){
  ADC0_ISC_R = 0x8;
}

int mapTempColor(float temp){
  if(temp < 17){
    return 1;
  } else if(temp >= 27){
    return 7;
  } else {
    return (int)(temp / 2.0 - 6.5);
  }
}

void setFreq(int f){
  //set System clock
  int SysDiv = 400.0 / f - 1.0;
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~(SYSCTL_RCC2_SYSDIV2_M + 
                   SYSCTL_RCC2_SYSDIV2LSB)) | (SysDiv << 22);
  //set baudrate
  double baudRate = f * 1000000 / (16.0 * BAUDRATE);
  UART1_IBRD_R = (int)baudRate;
  UART1_FBRD_R = (int)((baudRate - (int)baudRate) * 64.0 + 0.5);
  UART1_LCRH_R |= 0x60;
  //set timer
  setTime0(f * 1000000);
}

int sw1 = 0, sw2 = 0;

int main(){
  
  Timer0_Init();
  UART_Init();
  Port_Init();
  PLL_Init();
  ADC_Init();
  
  StartTimer0();
  
  while(1){
    
    if(!RxEmpty()){
      char c = ReadData();
      if(c == 'R'){ //Receiving data
        for(int i = 0; i < MAX_SLOT; i++){
          //Slot number
          int slotNum;
          while(RxEmpty()){};
          slotNum = ReadData();
          
          //Pill name
          char* pillName;
          do {
            while(RxEmpty()){};
            pillName = (char)ReadData();
          } while(pillName != '/0');
          
          //amount
          int amount;
          while(RxEmpty()){};
          amount = ReadData();
          
          //timeSize
          int timeSize;
          while(RxEmpty()){};
          timeSize = ReadData();
          
          //Times
          int time[MAX_TIMESET];
          int index = 0;
          for(int t = 0; t < timeSize; t++){
            while(RxEmpty()){};
            time[t] = ReadData();
          }
          
          //update pill
        }
      } 
    }
    
  }
}

void ADC0_Handler(){
  ClearADC0_Input();
  float temp = (147.5 - (247.5 * Read_ADC0()) / 4096);
  int color = mapTempColor(temp);
  setColor(color);
  
  char strTmp[32];
  snprintf(strTmp, sizeof(strTmp), "Temp: %.1f\n\r", temp);
  WriteString(strTmp);
}
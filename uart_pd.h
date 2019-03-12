#ifndef __UART_H_
#define __UART_H_

#define UART_READY 0
#define UPDATE_PILL 1

void pll_Init();

void uart1_Init();

void uart1_outchar(char c);

void uart1_outstring(char* s);

char uart1_inchar();

char uart1_inchar_non_block();

#endif // __UART_H_
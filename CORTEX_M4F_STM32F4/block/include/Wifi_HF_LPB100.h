#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_l3gd20.h"

void Wifi_config();
int toCmdmode();

int closeE();

int checkTCP();

int sendDataLeng(int);
int USART6_puts(char*);

int UART_cmp(char*);


#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_l3gd20.h"
#include "stdio.h"
#include "stdlib.h"

void Wifi_config(){
	


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);   // USART6_TX
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);  // USART6_RX

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART6, &USART_InitStructure); //USART2 init
	USART_Cmd(USART6, ENABLE);


}
static char* itoa(int value, char* result, int base)
{
	if (base < 2 || base > 36) {
		*result = '\0';
		return result;
	}
	char *ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while (value);

	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr-- = *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

int toCmdmode(){
	for(int i=0;i<1000;i++){}
	//USART6_puts("+++");
	USART6_puts("+");
	//for(int i=0;i<100;i++){}
	USART6_puts("+");   //put +++
	//for(int i=0;i<100;i++){}	
	USART6_puts("+");

//	USART6_puts("+");

	while(USART_GetFlagStatus(USART6, USART_FLAG_RXNE) == RESET);
	//char* str = itoa(USART_ReceiveData(USART6),str,10);
	uint16_t a = USART_ReceiveData(USART6);
	
	for(int i = 8 ; i>=0; i--){
		if(a&(1<<i))
			USART1_puts("1");
		else USART1_puts("0");
	}


	if(a ==0x0061){
		USART6_puts("a");
		//USART1_puts("inside");
	}
	else return 0;
	if(UART_cmp("+ok")){
		//USART1_puts("222"); 
		return 1;
	}
	else return 0;


}

int closeE(){

	USART6_puts("AT+E=off\r");   
	if(UART_cmp("+ok\r\n\r\n")) return 1;
	else return 0;

}

int checkTCP(){

	USART6_puts("AT+TCPLK\r");   
	if(UART_cmp("+ok=on\r\n\r\n")) return 1;
	else return 0;

}

int sendDataLeng(int size){

	char* s=itoa(size, s, 10);
	USART6_puts("AT+SEND\r");   //put +++
	USART6_puts(s);
	if(UART_cmp("+ok\r\n\r\n")) return 1;
	else return 0;
}

void WifiTest(){

	Wifi_config();
	USART1_puts("config ok\r\n");
	if(!toCmdmode()) USART1_puts("error occurs when switch to command mode\r\n");
	
	else{
		USART1_puts("command mode ok\r\n");
		if(!closeE()) USART1_puts("error occurs when closing E\r\n");
		else USART1_puts("closeE\r\n");

		while(!checkTCP()); // repeat until TCP connect
		USART1_puts("TCP connect OK!\r\n");
	}

	
	while(1)
		USART6_puts("AT+L\r");
	
	
}
void USART6_puts(char* s)
{
    while(*s) {
        while(USART_GetFlagStatus(USART6, USART_FLAG_TXE) == RESET);
	USART_SendData(USART6, *s);
        s++;
    }
}

int UART_cmp(char* str){
	while(*str){
		while(USART_GetFlagStatus(USART6, USART_FLAG_RXNE) == RESET);
		//USART1_puts("dbg-cmp\r\n");
		//USART_SendData(USART1, USART_ReceiveData(USART6));
		if(USART_ReceiveData(USART6)!=*str)	return 0;
		else{
			str++;
		}

	}
	return 1;

}


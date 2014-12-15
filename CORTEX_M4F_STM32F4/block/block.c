#include "game.h"
#include "main.h"
#include "stm32f4xx.h"
#include "shell.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_l3gd20.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "semphr.h"


#define SCANTIMEOUT 100000
#define BLOCKMAX 100

/* test */
uint8_t block[BLOCKMAX];


int blockTotal=0;

/* test */



void scanBlock(){

	USART1_puts("\n\rscanning start\n\r");
       	while(USART_GetFlagStatus(USART6, USART_FLAG_TXE) == RESET);
	
	USART_SendData(USART6, 0x31);
	
	
	while(USART_GetFlagStatus(USART6, USART_FLAG_RXNE) == RESET);
	
        		uint8_t a = USART_ReceiveData(USART6);


        	int i = 0; 
       	/*scan until t == 0x00 */
        	while(1){
        		
        		//USART_SendData(USART1, '3');
        		while(USART_GetFlagStatus(USART6, USART_FLAG_RXNE) == RESET);
        		block[i] = USART_ReceiveData(USART6);
        		USART_SendData(USART1, block[i]+48);
        		if(block[i]==0x00) break; // last ' } ' receive

        		i++;

        	}
        	USART1_puts("scan finished");
        	blockTotal=i;

}

void showCode(){

	int tabNum=0;

	USART1_puts("\r\n\r\n");

	for(int i=0; i<=blockTotal; i++){

		

		//control
		if((block[i]&0x80)==0x80){  

			if(tabNum > 0) putTab(tabNum);
			tabNum++;
			
			switch (block[i]) {

				case 0x80:  //while

					USART1_puts("while(");
					i++;
					while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
					USART_SendData(USART1, block[i]+48);
					USART1_puts("){ \r\n");
					
					break;

				case 0x81: //if

					USART1_puts("if(");
					i++;
					while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
					USART_SendData(USART1, block[i]+48);
					USART1_puts("){ \r\n");
					
					break;

				case 0x82: //else

					USART1_puts("else{\r\n");
					break;

				default :

					USART1_puts("block config error!\r\n");
					break;

			}
		}
		else if(block[i]==0x01||block[i]==0x00){
			
			tabNum--;
			if(tabNum > 0) putTab(tabNum);
			USART1_puts("}\r\n");
			

		}
		else{
			if(tabNum > 0) putTab(tabNum);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
			USART_SendData(USART1, block[i]+48);
			USART1_puts(";\r\n");
		}

		


	}



}


void putTab(int n){

	for(int j=0; j<n; j++ ){

		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, '\t'); //tab
	}

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


/**********************************UART***********************************************/

void RCC_Configuration(void)
{
      /* --------------------------- System Clocks Configuration -----------------*/
      

       RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

/* USART1 clock enable */
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

       
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
      /* GPIOA clock enable */

}
 
/**************************************************************************************/
 
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	 /*-------------------------- GPIO Configuration(UART 1) ----------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*-------------------------- GPIO Configuration(UART 6) ----------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Connect USART pins to AF */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);   // USART1_TX
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);  // USART1_RX

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);   // USART2_TX
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);  // USART2_RX
}
 
/**************************************************************************************/
 
void USART_Configuration(void)
{
    USART_InitTypeDef USART_InitStructure;

    /* USARTx configuration ------------------------------------------------------*/
    /* USARTx configured as follow:
     *  - BaudRate = 9600 baud
     *  - Word Length = 8 Bits
     *  - One Stop Bit
     *  - No parity
     *  - Hardware flow control disabled (RTS and CTS signals)
     *  - Receive and transmit enabled
     */
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART6, &USART_InitStructure); //USART2 init
	USART_Cmd(USART6, ENABLE);

	USART_Init(USART1, &USART_InitStructure);  //USART1 init
	USART_Cmd(USART1, ENABLE);

	

}

void USART1_puts(char* s)
{
    while(*s) {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, *s);
        s++;
    }
}

/**************************************************************************************/


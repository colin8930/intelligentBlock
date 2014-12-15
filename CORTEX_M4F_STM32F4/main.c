/**
  ******************************************************************************
  * @file    Template/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "shell.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "semphr.h"
/** @addtogroup Template
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

volatile xSemaphoreHandle serial_tx_wait_sem = NULL;
volatile xQueueHandle t_queue = NULL;
volatile xQueueHandle serial_rx_queue = NULL;



xTaskHandle xcmdTask;  			//UART
xTaskHandle xUartTask;			//game



int last_command=1;
int SUART_Return=1;  //shell 
int GUART_Return=0;  //game
char recv_byte();

void
prvInit()
{
	

	//Button
	STM_EVAL_PBInit( BUTTON_USER, BUTTON_MODE_GPIO );


	//UART
	RCC_Configuration();
  	GPIO_Configuration();
  	USART_Configuration();
 	USART1_puts("Welcome !\r\n");
}



char recv_byte()
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
        char t = USART_ReceiveData(USART1);
	return t;
}

ssize_t stdin_read(void * buf, size_t count) {
    int i=0, endofline=0, last_chr_is_esc;
    char *ptrbuf=buf;
    char ch;

	const portTickType xDelay = 1;
    while(i < count&&endofline!=1){
	ptrbuf[i]=recv_byte();
	switch(ptrbuf[i]){
		case '\r':
		case '\n':
			ptrbuf[i]='\0';
			endofline=1;
			break;
		case '[':
			if(last_chr_is_esc){
				last_chr_is_esc=0;
				ch=recv_byte();
				if(ch>=1&&ch<=6){
					ch=recv_byte();
				}
				continue;
			}
		case ESC:
			last_chr_is_esc=1;
			continue;
		case BACKSPACE:
			last_chr_is_esc=0;
			if(i>0){
				
				USART_SendData(USART1, '\b');
				vTaskDelay(xDelay );
				USART_SendData(USART1, ' ');
				vTaskDelay(xDelay );
				USART_SendData(USART1, '\b');
				--i;
			}
			continue;
		default:
			last_chr_is_esc=0;
	}
	USART_SendData(USART1, ptrbuf[i]);
	
	++i;
    }
    return i;
}

void command_prompt(void *pvParameters)
{
	
	char buf[128];
	char *argv[20];
        	char hint[] = "block: ";

	USART1_puts("\rWelcome !\r\n");
	while(1){
		
   		USART1_puts(hint);
		
		stdin_read(buf, 127);
	
		int n=parse_command(buf, argv);

		/* will return pointer to the command function */
		cmdfunc *fptr=do_command(argv[0]);
		
		if(fptr!=NULL){						
			fptr(n, argv);
			}	
		else
			USART1_puts("\r\ncommand not found.\r\n");
	}

}


/*
static void EventTask1( void *pvParameters )
{
	while( 1 ){
		 EventHandler1();
	}
}
*/

//Main Function
int main(void)
{
	prvInit();

	/* Create the queue used by the serial task.  */
	vSemaphoreCreateBinary(serial_tx_wait_sem);
	/* Add for serial input 
	 * Reference: www.freertos.org/a00116.html */
	serial_rx_queue = xQueueCreate(1, sizeof(char));
	


	
		
		 
	xTaskCreate(command_prompt,  (signed char *) "command_prompt",   512 , NULL, tskIDLE_PRIORITY + 3, &xcmdTask);		
		

	vTaskStartScheduler();	
		
	//Call Scheduler
	//vTaskStartScheduler();
}

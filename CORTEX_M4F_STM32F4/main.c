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
#include "Wifi_HF_LPB100.h"
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

char timeStr[10];
xTaskHandle xcmdTask;  			//UART

char recv_byte();
void USART3_puts(char* s)
{
    while(*s) {
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	USART_SendData(USART3, *s);
        s++;
    }
}

void
prvInit()
{
	

	//Button
	STM_EVAL_PBInit( BUTTON_USER, BUTTON_MODE_GPIO );
	//UART
	RCC_Configuration();
  	GPIO_Configuration();
  	USART_Configuration();

  	//button
  	STM_EVAL_PBInit( BUTTON_USER, BUTTON_MODE_GPIO );


}



//Main Function
int main(void)
{
	//RTC_setting();
	prvInit();
	initialize_RTC();
	//LCD_Configuration();
	/* Create the queue used by the serial task.  */
	vSemaphoreCreateBinary(serial_tx_wait_sem);
	/* Add for serial input 
	 * Reference: www.freertos.org/a00116.html */
	serial_rx_queue = xQueueCreate(1, sizeof(char));	
	/*
	xTaskCreate(command_prompt,  (signed char *) "command_prompt",   512 , NULL, tskIDLE_PRIORITY + 1, &xcmdTask);			

	vTaskStartScheduler();	*/

	while(!STM_EVAL_PBGetState(BUTTON_USER));

	scanBlock();
	run();
		
	//Call Scheduler
	//vTaskStartScheduler();
}

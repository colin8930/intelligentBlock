#include "main.h"
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "FreeRTOS.h"
#include "task.h"
#include "global.h"



#define SCANTIMEOUT 100000
#define BLOCKMAX 100
#define M74HC165_PORT		GPIOB
#define M74HC165_PORT_RCC	RCC_AHB1Periph_GPIOB
#define M74HC165_ENABLE		GPIO_Pin_0
#define M74HC165_LOAD		GPIO_Pin_1
#define M74HC165_CLOCK		GPIO_Pin_2
#define M74HC165_DATA		GPIO_Pin_3

uint8_t block[BLOCKMAX];

char Str[10];

int blockTotal=0;
int a=0;
int b=0;
int c=0;
int ifCount=0;
int elseCount=0;


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
int varAssign(uint8_t in){    //for condition
	switch(in){
		

		case VARA: ;
			return a;
			break;
		case VARB: ;
			return b;
			break;
		case VARC: ;
			return c;
			break;
		default :
			return 0;  // should not be here
			break;		
	}
	return 0;
}

int cal(uint8_t con, uint8_t* nextR){

	switch(con){
		
		case ADD:
			
			if((block[*nextR+2]&0x40)==0x40){

				switch(block[*nextR+1]){

				case VARA:					
					a+=varAssign(block[*nextR+2]);
					break;
				case VARB:
					b+=varAssign(block[*nextR+2]);
					break;
				case VARC:
					c+=varAssign(block[*nextR+2]);
					break;	
				default:
					break;

				}
			}
			else{
				switch(block[*nextR+1]){

				case VARA:
					a+=block[*nextR+2];
					return *nextR+3;
					break;
				case VARB:
					b+=block[*nextR+2];
					return *nextR+3;
					break;
				case VARC:
					c+=block[*nextR+2];
					return *nextR+3;
					break;	
				default:
					break;
				}
			}
			break;

		case MINUS:
			if((block[*nextR+2]&0x40)==0x40){

				switch(block[*nextR+1]){

				case VARA:
					a-=varAssign(block[*nextR+2]);
					break;
				case VARB:
					b-=varAssign(block[*nextR+2]);
					break;
				case VARC:
					c-=varAssign(block[*nextR+2]);
					break;	
				default:
					break;

				}
			}
			else{

				switch(block[*nextR+1]){

				case VARA:
					a-=block[*nextR+2];
					break;
				case VARB:
					b-=block[*nextR+2];
					break;
				case VARC:
					c-=block[*nextR+2];
					break;	
				default:
					break;
				}
			break;
		}
			
	}


}

int det_condition(uint8_t con, uint8_t* nextR){
	int x, y=0;
	//USART1_put(con);
	switch(con){

		case LDRPUSH: ;
			USART1_put(con);
			if(getSensor(con)) {
				*nextR=*nextR+2;
				return 1;
			}
			else return 0;		
			break;
		case IFAHEAD: ;
			USART1_put(con);
			if(getSensor(con)) {
				*nextR=*nextR+2;
				return 1;
			}
			else return 0;		
			break;
		case IFLEFT: ;
			USART1_put(con);
			if(getSensor(con)) {
				*nextR=*nextR+2;
				return 1;
			}
			else return 0;		
			break;
		case IFRIGHT: ;
			USART1_put(con);
			if(getSensor(con)) {
				*nextR=*nextR+2;
				return 1;
			}
			else return 0;		
			break;

		case GREATER: ;
			
			x=varAssign(block[*nextR+2]);
			if((block[*nextR+3]&0x40)==0x40){
				y=varAssign(block[*nextR+3]);
			}
			else y=block[*nextR+3];
			if(x>y) {
				*nextR=*nextR+4;
				return 1;
			}
			else{
				return 0;				
			}


		case LESS: ;

			
			x=varAssign(block[(*nextR)+2]);

			if((block[*nextR+3]&0x40)==0x40){
				y=varAssign(block[*nextR+3]);
			}
			else y=block[*nextR+3];
			
			
			if(x<y) {
				*nextR=*nextR+4;
				return 1;
			}
			else{
				return 0;				
			}

		case EQUAL: ;

			x=varAssign(block[*nextR+2]);
			
			if(block[*nextR+3]&0x40==0x40){
				y=varAssign(block[*nextR+3]);
			}
			else y=block[*nextR+3];

			if(x==y) {
				*nextR=*nextR+4;	
				return 1;
			}
			else{
				return 0;				
			}

	}

}
void scanBlock(){

	/* init the GPIO */
	

	/* read shift register first*/
	blockTotal=0;
	USART1_puts("\n\rscanning start\n\r");
	M74HC165_Init();
	readSRs();
#if 0
       	while(USART_GetFlagStatus(USART6, USART_FLAG_TXE) == RESET);
	
	USART_SendData(USART6, 0x31);
	
	
	while(USART_GetFlagStatus(USART6, USART_FLAG_RXNE) == RESET);
	
        	uint8_t a = USART_ReceiveData(USART6);


        	int i = blockTotal;
       	/*scan until t == 0x00 */
        	while(1){
        		
        		//USART_SendData(USART1, '3');
        		while(USART_GetFlagStatus(USART6, USART_FLAG_RXNE) == RESET);
        		block[i] = USART_ReceiveData(USART6);
        		USART_SendData(USART1, block[i]+48);
        		if(block[i]==0x00) break; // last ' } ' receive

        		i++;

        	}

        	blockTotal=i;
#endif
}

char * _byteToStr(uint8_t oneByte)	// Hexadecimal to Decimal'string 
{
	
	*Str ='0';
	*(Str+1) = 'x';

	uint8_t fw = (oneByte>>4);
	if(fw<10) *(Str+2) = fw+'0';
	else	*(Str+2)=(fw-0xA)+'A';
	
	uint8_t sw = oneByte&0x0F;

	if(sw<10) *(Str+3) = sw+'0';
	else	*(Str+3)=(sw-0xA)+'A';
	*(Str+4)='\r';
	*(Str+5)='\n';
	return Str;
}

void run(){

	USART1_puts("\r\n");
	int run = 0;
	while( (run=runCode(run))!=0);
	USART1_puts("running finished\r\n");

}

int runCode(int run){

	while(block[run]!=0){

		if((block[run]&OPCODE)==OPCODE){   //special block
			int next = run;
			USART1_put(next+0x30);
			switch(block[run]){

				case REPEAT: ;
					uint8_t repeatTime = block[run+1];
					
					for(int i = 0; i < repeatTime; i++){

						next=runCode(run+2);
					}

					return next;
					break;

				case REPEATUNTIL: ;    //while
					{
					uint8_t whileCond = block[run+1];
					int torun=next;
					if(whileCond<0x40||whileCond>0x43)
					{
						sendCmd(ERROR);
						return 0;
					}

					while(!det_condition(whileCond, &next)){

						// if it return false, run code until  the bracket of while, while() {    " } " .
						ifCount++;
						runCode(next);
						next=torun;
					}
					elseCount++;
				}
					return runCode(norunCode(next+1) );
					break;
				case WAITUNTIL: ;
					{
					uint8_t waitCond = block[run+1];
					int torun=next;
					if(waitCond<0x40||waitCond>0x43)
					{
						sendCmd(ERROR);
						return 0;
					}
					while(!det_condition(waitCond, &next)){
						next=torun;
						//runCode(next);
						
					}
					}
					return runCode(next);
					break;
				case IF: ;
					uint8_t ifCond = block[run+1];
					if(ifCond<0x40||ifCond>0x43)
					{
						sendCmd(ERROR);
						return 0;
					}
					if(det_condition(ifCond, &next)){

						ifCount=1;
						next=runCode(next);

						if(block[next]==ELSE){
							return runCode(norunCode(next+1) );					
						}
						return runCode(next);
					} 
					else{
						
						next=norunCode(run+1);
						if(block[next]==ELSE){
							elseCount++;
							return runCode(next+1);
						}
						else return runCode(next);
					}

					break;
				case WAIT: ;
					uint8_t min = block[run+1];
					uint8_t sec = block[run+2];
					reset_alarm();
					set_alarm_time(min, sec, 'a');
					while(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==RESET); // wait until time out
					USART1_puts("time out");
					return runCode(next+3);


				default:
					break;

			}

		}


		else if(block[run]==0x01&&ifCount>0){
			ifCount--;
			return run+1;
		} 
		else if(block[run]==0x01&&elseCount>0){
			elseCount--;
			return run+1;
		} 
		else if(block[run]!=0x00&&block[run]!=0x01){
			if(block[run]>=0x40||block[run]<=0x43)
			{
				sendCmd(ERROR);
				return 0;
			}
			int ack=sendCmd(block[run]);
			if(!ack) USART1_puts("error occurs when the device doing the command\r\n");
			else USART1_puts("send OK\r\n");
			return 	runCode(run+1);
		}
		else return run+1;


	} 
	return 0;
}

int norunCode(int run){  //for if else

	int  count=1; 
	int running = run;
	while(count>0){

		if((block[running]&OPCODE)==OPCODE){   //special block
			
			switch(block[running]){

				case REPEAT: ;
					count++;
				break;
					
				case IF: ;
					count++;
				break;
				
				case ELSE: ;
					count++;
					break;
				
				case PRINT: ;
					break;

				case ADD: ;					
					break;

				case MINUS: ;					
					break;

				default:
					break;

			}

		}


		else if(block[running]==BRACKET){
			count--;
			
		} 
		running++;

	} 
	return running;
}
char* cmdtoStr(uint8_t cmd){

	switch(cmd){

		case LDRPUSH: ;
			return "LDRPUSH";	
			break;
		case IFAHEAD: ;
			return "IFAHEAD";	
			break;
		case IFLEFT: ;
			return "IFLEFT";	
			break;
		case IFRIGHT: ;
			return "IFRIGHT";	
			break;
		case MOVEFORWARD : ;
			return "IFRIGHT";	
			break;	
		case TURNRIGHT: ;
			return "TURNRIGHT";	
			break;	
		case TURNLEFT: ;
			return "TURNLEFT";	
			break;	
		case ALARMON: ;
			return "ALARMON";	
			break;	
		case ALARMOFF: ;
			return "ALARMOFF";	
			break;	

		default:

			break;



	}


}
void sendcode(){

	int tabNum=0;
	USART1_puts("\r\n");
	for(int i=0; i<=blockTotal; i++){		

		//control
		if((block[i]&OPCODE)==OPCODE){  

			if(tabNum > 0) putTab(tabNum);
			
			
			switch (block[i]) {

				case WAIT:  //while
					USART1_puts("wait(");
					i++;
					char* waittime;
					waittime=itoa(block[i], waittime ,10);
					USART1_puts(waittime);
					i++;
					waittime=itoa(block[i], waittime ,10);;
					USART1_puts(waittime);
					//while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
					//USART_SendData(USART1, block[i]+48);
					USART1_puts("); \r\n");
					
					break;

				case REPEAT:  //while
					tabNum++;
					USART1_puts("repeat(");
					i++;
					char* string=" ";
					string=itoa(block[i], string ,10);
					USART1_puts(string);
					//while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
					//USART_SendData(USART1, block[i]+48);
					USART1_puts("){ \r\n");
					
					break;

				case REPEATUNTIL:  //while
					tabNum++;
					USART1_puts("repeat until(");
					i++;
					USART1_puts(cmdtoStr(block[i]));
					USART1_puts("){ \r\n");
					
					break;

				case IF: //if
					tabNum++;
					USART1_puts("if(");
					i++;
					USART1_puts(cmdtoStr(block[i]));		
					
					USART1_puts("){ \r\n");
					
					break;

				case ELSE: //else
					tabNum++;
					USART1_puts("else{\r\n");
					break;
				case PRINT: ;
					USART1_puts("print(\"");
						i++;
					if(block[i]==HELLOWORLD){

						USART1_puts("Hello world!");
						i++;
					}
					else if(block[i]==TEST){

						USART1_puts("TEST");
						i++;
					}
					else{
						while(block[i]!=0x01){
							USART1_put(block[i]);
							i++;
						}

					}		
					
					USART1_puts("\");\r\n");

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
			USART1_puts(cmdtoStr(block[i]));
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


uint8_t readSR(){

	uint8_t result=0;
	for(int i = 7; i >=0; i--){

		
		//result|=(<<i);
		if(M74HC165_PORT->IDR&M74HC165_DATA) result|= (1<<i);
		GPIO_ResetBits(M74HC165_PORT, M74HC165_CLOCK);
		GPIO_SetBits(M74HC165_PORT, M74HC165_CLOCK);
		

	} 
	return result;


}

/* read shift register */
void readSRs(){ 

	/*load the parallel data*/
	GPIO_ResetBits(M74HC165_PORT, M74HC165_LOAD);
	
	GPIO_SetBits(M74HC165_PORT, M74HC165_LOAD);
	
	/*enable the clock */
	GPIO_SetBits(M74HC165_PORT, M74HC165_CLOCK);	
	GPIO_ResetBits(M74HC165_PORT, M74HC165_ENABLE);
	
	/* read a byte*/
	uint8_t incomming=readSR();
	USART1_puts(_byteToStr(incomming));
	while(incomming!=0){

		block[blockTotal]=incomming;
		blockTotal++;
		incomming = readSR();
		USART1_puts(_byteToStr(incomming));

	}
	
	GPIO_SetBits(M74HC165_PORT, M74HC165_ENABLE);

}



/**********************************74HC165*******************************/
void M74HC165_Init(void)
{
	/* set up shift register pin */
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(M74HC165_PORT_RCC, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = (M74HC165_CLOCK|M74HC165_LOAD|M74HC165_ENABLE);
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(M74HC165_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = M74HC165_DATA;
	GPIO_Init(M74HC165_PORT, &GPIO_InitStructure);

	GPIO_SetBits(M74HC165_PORT,M74HC165_LOAD);
	GPIO_SetBits(M74HC165_PORT,M74HC165_ENABLE);

}



/**********************************UART***********************************************/

void RCC_Configuration(void)
{
      /* --------------------------- System Clocks Configuration -----------------*/
      

       RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
       RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

/* USART1 clock enable */
      //RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

       
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

     //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
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


	/*-------------------------- GPIO Configuration(UART 3) ----------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOD, &GPIO_InitStructure);



	/*-------------------------- GPIO Configuration(UART 6) ----------------------------*/
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	//GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Connect USART pins to AF */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);   // USART1_TX
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);  // USART1_RX

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART1);   // USART3_TX
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART1);  // USART3_RX

	//GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);   // USART2_TX
	//GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);  // USART2_RX
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

	//USART_Init(USART6, &USART_InitStructure); //USART2 init
	//USART_Cmd(USART6, ENABLE);

	USART_Init(USART1, &USART_InitStructure);  //USART1 init
	USART_Cmd(USART1, ENABLE);

	USART_InitStructure.USART_BaudRate = 38400;
	USART_Init(USART3, &USART_InitStructure);  //USART3 init (for bt)
	USART_Cmd(USART3, ENABLE);

	

}

void USART1_puts(char* s)
{
    while(*s) {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, *s);
        s++;
    }
}

void USART1_put(uint8_t s)
{

        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, s);

}



/**************************************************************************************/


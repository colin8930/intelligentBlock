#include "main.h"
#include "stm32f4xx.h"
#include "shell.h"
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


//uint8_t block[BLOCKMAX];
//uint8_t block[BLOCKMAX]={REPEAT, 10, IF, LESS, X, 5, PRINT, HELLOWORLD, BRACKET, BRACKET, ELSE , PRINT, TEST,BRACKET, BRACKET, ADD, X, 1, BRACKET,  END};
uint8_t block[BLOCKMAX]={ ALARMON, PRINT,HELLOWORLD,BRACKET, REPEAT, 10, 
	IF, IFAHEAD, MOVEFORWARD, BRACKET, IF, IFLEFT, TURNLEFT,BRACKET, IF,IFRIGHT, TURNRIGHT,BRACKET, BRACKET,ALARMOFF, END};
//uint8_t block[BLOCKMAX]={WAIT, 0x00, 0x10, PRINT, HELLOWORLD, END};
int state = 0;  //not used now

/*
	0: blockly

*/


/*test*/
/*
	print("test");
	repeat(10){
		print(Hello World!);
	}


*/

char Str[10];
int blockTotal=14;
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
	switch(con){

		case IFAHEAD: ;

			if(getSensor('B')) {
				*nextR=*nextR+2;
				return 1;
			}
			else return 0;		
			break;

		case IFLEFT: ;

			if(getSensor('C')) {
				*nextR=*nextR+2;
				return 1;
			}
			else return 0;
			break;
			
		case IFRIGHT: ;

			if(getSensor('D')) {
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
	USART1_put(block[0]+48);
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
        	#endif
        	USART1_puts("scan finished\n\r");
        	//blockTotal=i;

}

char * _byteToStr(uint8_t oneByte)	// Hexadecimal to Decimal'string 
{
	
	*Str ='0';
	*(Str+1) = 'x';

	uint8_t fw = (oneByte>>4);
	if(fw<10) *(Str+2) = fw+'0';
	else {
		switch(fw){
			case 0xA: ;
				*(Str+2)='A';
				break;
			case 0xB: ;
				*(Str+2)='B';
				break;
			case 0xC: ;
				*(Str+2)='C';
				break;
			case 0xD: ;
				*(Str+2)='D';
				break;
			case 0xE: ;
				*(Str+2)='E';
				break;
			case 0xF: ;
				*(Str+2)='F';
				break;
			default:
				break;

		}
	}
	uint8_t sw = oneByte&0x0F;

	if(sw<10) *(Str+3) = sw+'0';
	else {
		switch(sw){
			case 0xA: ;
				*(Str+3)='A';
				break;
			case 0xB: ;
				*(Str+3)='B';
				break;
			case 0xC: ;
				*(Str+3)='C';
				break;
			case 0xD: ;
				*(Str+3)='D';
				break;
			case 0xE: ;
				*(Str+3)='E';
				break;
			case 0xF: ;
				*(Str+3)='F';
				break;
			default:
				break;

		}
	}
	*(Str+4)='\r';
	*(Str+5)='\n';
	return Str;
}


void showCode(){

	int tabNum=0;

	USART1_puts("\r\n\r\n");
	#if 0
	for(int i=0; i<=blockTotal; i++){

		

		//control
		if((block[i]&OPCODE)==OPCODE){  

			if(tabNum > 0) putTab(tabNum);
			
			
			switch (block[i]) {

				case REPEAT:  //while
					tabNum++;
					USART1_puts("repeat(");
					i++;
					char* string=" ";
					string=itoa(block[i], string ,10);
					USART1_puts("10");
					//while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
					//USART_SendData(USART1, block[i]+48);
					USART1_puts("){ \r\n");
					
					break;

				case IF: //if
					tabNum++;
					USART1_puts("if(");
					i++;
					USART1_puts("X");
					i++;
					USART1_puts("<");
					i++;
					USART1_puts("5");
					
					
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
				case ADD: ;

					USART1_puts("add(");
					i++;
					USART1_puts("X, ");
					i++;
					USART1_put(block[i]+0x30);
					i++;
					USART1_puts(");\r\n");
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

	#endif
	int i = 0;
	
	while(block[i]!=0) USART1_puts(_byteToStr(block[i++]));

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
			switch(block[run]){

				case REPEAT: ;
					uint8_t repeatTime = block[run+1];
					
					for(int i = 0; i < repeatTime; i++){

						next=runCode(run+2);
					}

					return next;
					break;

				case REPEATUNTIL: ;    //while

					uint8_t whileCond = block[run+1];
					int torun=next;
					
					while(det_condition(whileCond, &next)){

						// if it return TRUE, run code until  the bracket of while, while() {    " } " .
						ifCount++;
						runCode(next);
						next=torun;
					}
					elseCount++;
					return runCode(norunCode(next+1) );
					break;
				case IF: ;
					uint8_t ifCond = block[run+1];
					
					if(det_condition(ifCond, &next)){

						ifCount++;
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


				/*
				case ELSE: ;


				break;

				*/
				case PRINT: ;
					int running = run+1;
					if(block[running]==HELLOWORLD){

						USART1_puts("Hello world!\r\n");
						running++;
					}
					else if(block[running]==TEST){

						USART1_puts("TEST\r\n");
						running++;
					}

					else{
						while(block[running]!=BRACKET){
						USART1_put(block[running]);
						running++;
						}
					}
					

					return runCode(running+1);  //next of 0x01
					break;
				case ADD: ;
					next=cal(ADD, &next);
					return runCode(next);
					break;
				case MINUS: ;
					next=cal(MINUS, &next);
					return runCode(next);
					break;

				case WAIT: ;
					uint8_t min = block[run+1];
					uint8_t sec = block[run+2];
					reset_alarm();
					set_alarm_time(min, sec, 'a');
					while(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==RESET); // wait until time out
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

void sendcode(){

	for(int i=0; i<blockTotal; i++ ){
		USART3_put(block[i]);
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
	USART1_put(incomming+48);
	while(incomming!=0){

		block[blockTotal]=incomming;
		blockTotal++;
		incomming = readSR();

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


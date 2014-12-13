#include "shell.h"
#include <stddef.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include "queue.h"
#include "semphr.h"
#include "main.h"
#include "game.h"


typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;

extern volatile xQueueHandle t_queue;

void scan_command(int, char **);
void show_command(int, char **);



#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={

	MKCL(scan, "scan"),
	MKCL(show, "show")

};

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}



void scan_command(int n, char *argv[]){
		
	
	scanBlock();

}

void show_command(int n, char *argv[]){

	showCode();

}

cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}


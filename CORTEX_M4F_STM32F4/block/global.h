
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GLOBAL_H
#define __GLOBAL_H

#define OPCODE 0x80
#define REPEAT 0x80
#define IF 0x81
#define ELSE 0x82
#define PRINT 0x83
#define REPEATUNTIL 0x84
#define UINT8 0x85
#define UINT16 0x86
#define GREATER 0x87
#define LESS 0x88
#define EQUAL 0x89
#define ADD 0x8A
#define MINUS 0x8B
#define TIMER 0x8C
#define COUNTER 0x8D
#define WAIT 0x8E
#define WAITUNTIL 0x8F

#define VARA 0xF0
#define VARB 0xF1
#define VARC 0xF2

#define END 0x00
#define BRACKET 0x01
#define MOVEFORWARD 0x05
#define TURNRIGHT 0x06
#define TURNLEFT 0x07
#define ALARMON 0x0B
#define ALARMOFF 0x0C
#define SNAPSHOT 0x08
#define PLAYSOUND1 0x09
#define PLAYSOUND2 0x0A
#define PLAYSOUND3 0x0B
#define PLAYSOUND4 0x0C
#define PLAYSOUND5 0x0D
#define PLAYSOUND6 0x0E
#define PLAYSOUND7 0x0F
#define PLAYSOUND8 0x10
#define PLAYSOUND9 0x11


#define IFAHEAD 0x40
#define IFRIGHT 0x41
#define IFLEFT 0x42
#define LDRPUSH 0x43

#define HELLOWORLD 0x0B
#define TEST 0x0C

#define ERROR 0x00

#endif 
#include <stdio.h>
#include <stdarg.h>

#include "spider.h"
#include "serial.h"


void print(char* msg,...){
//	char *p=&buf[0];
//	va_list va;
//
//	va_start(va,msg);
//	vsprintf(buf,msg,va);
//	va_end(va);
//
//	while(*p != 0){
//		SER_PutChar(*p);
//		p++;
//	}
	while(*msg != 0){
		SER_PutChar(*msg);
		msg++;
	}	// 这样还是能正常工作诶...
}


#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "ioport.h"

#define testbit(flags, bit) ((flags) & (1 << (bit)))

#define serial_base  0x3f8
#define serial_data  serial_base + 0
#define serial_ier   serial_base + 1
#define serial_lcr   serial_base + 3
#define serial_lsr   serial_base + 5
#define serial_lsr_canwrite 5

static inline void write_serial(char c){
	while(!testbit(in8(serial_lsr),serial_lsr_canwrite));
	out8(serial_base,c);
}

static inline void setup_serial(){
	out8(serial_lcr, 0x81);
	out8(serial_data, 1); //divisor =
	out8(serial_ier, 0); //           1

	out8(serial_lcr, 0x01);
	out8(serial_ier, 0); //no interrupts from serial port
}

void puts(char *s){
	while(*s)
		write_serial(*(s++));
	write_serial('\n');
}

void writelnInt(int a){
	char s[10];
	int l=0;
	do{
		s[l++]=a%10, a/=10;
	}while(a);
	for(int i=l-1;i>=0;--i)
		write_serial(s[i]+'0');
	write_serial('\n');
}



#endif /* __SERIAL_H__ */
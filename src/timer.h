#ifndef __TIMER_H__
#define __TIMER_H__

#include "ioport.h"
#include "interrupt.h"
#include "pic.h"

#define timer_control_port 0x43
#define timer_data_port0 0x40
#define timer_delimiter 0xFFFF

const int TIMER_FREQ = 1193180;
const int DECSECS = 3;

void timer_int2(){
	static int counter=0, big_counter=0;
	++counter;
	if (counter*timer_delimiter*10>=DECSECS*TIMER_FREQ){
		writelnInt(big_counter);
		++big_counter;
		counter=0;
	}
	eoi(0);
}

void timer_int();
__asm__("timer_int:\
		push %rax; push %rcx; push %rdx; push %rdi; push %rsi; push %r8; push %r9; push %r10; push %r11; cld;\
		call timer_int2;\
		pop %r11; pop %r10; pop %r9; pop %r8; pop %rsi; pop %rdi; pop %rdx; pop %rcx; pop %rax; iretq;");

static inline void setup_timer(){
	out8(timer_control_port,0x34);
	out8(timer_data_port0,timer_delimiter&0xff);
	out8(timer_data_port0,timer_delimiter>>8);
	add_interrupt_gate(m_interrupt_base+0,timer_int);
}


#endif /* __PIC_H__ */
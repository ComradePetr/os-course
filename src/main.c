#include "ioport.h"
#include "serial.h"
#include "pic.h"
#include "interrupt.h"
#include "timer.h"

void main(void)
{
	setup_serial();
	puts("final countup");
	
	setup_pic();
	setup_idt();
	setup_timer();
	__asm__ volatile ("sti");

	while (1);
}

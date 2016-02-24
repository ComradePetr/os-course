#ifndef __PIC_H__
#define __PIC_H__

#include "ioport.h"

#define slave_number 2
#define m_command_reg 0x20
#define m_data_reg  0x21
#define m_interrupt_base 0x20
#define s_command_reg 0xa0
#define s_data_reg  0xa1
#define s_interrupt_base 0x28
#define command_eoi 0x20

static inline void setup_pic(){
	out8(m_command_reg,0x11);
	out8(m_data_reg,m_interrupt_base);
	out8(m_data_reg,BIT(slave_number));
	out8(m_data_reg,1);
	out8(s_command_reg,0x11);
	out8(s_data_reg,s_interrupt_base);
	out8(s_data_reg,slave_number);
	out8(s_data_reg,1);
}

static inline void eoi(int slave){
	if(slave)
		out8(s_command_reg, command_eoi);
	out8(m_command_reg, command_eoi);
}

#endif /* __PIC_H__ */
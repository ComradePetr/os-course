#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>
#include "memory.h"

struct idt_ptr {
	uint16_t size;
	uint64_t base;
} __attribute__((packed));

static inline void set_idt(const struct idt_ptr *ptr)
{ __asm__ volatile ("lidt (%0)" : : "a"(ptr)); }


#define IDT_SIZE 256
static struct idt_ptr pos;
struct idt_node{
    uint16_t offset_low, segment_selector;
    uint8_t ist, flags;
    uint16_t offset_mid;
    uint32_t offset_high, reserved;
} __attribute__((packed)) idt[IDT_SIZE];

void add_interrupt_gate(int id, void f()){
	struct idt_node x;
	uint64_t ptr=(uint64_t)f;
	x.segment_selector=KERNEL_CODE;
	x.offset_low=ptr&0xffff;
	x.offset_mid=(ptr>>16)&0xffff;
	x.offset_high=(ptr>>32);
	x.ist=0;
	x.reserved=0;
	x.flags=0x8E;
	idt[id]=x;
}

static inline void setup_idt(){
	pos.size=IDT_SIZE*sizeof(struct idt_node)-1, pos.base=(uint64_t)idt;
	set_idt(&pos);
}

#endif /*__INTERRUPT_H__*/

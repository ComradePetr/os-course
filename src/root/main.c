#include <stdint.h>
#include "../system_call.h"

void main(void){
	struct args0_s args0={
		.to_print = "Test message from userspace"
	};
	uint64_t id = 0, args = (uint64_t)&args0;
	__asm__ volatile (
		"movq %0, %%rax;"
		"movq %1, %%rbx;"
		"int $48;"
		:
		: "m"(id), "m"(args)
		: "rax", "rbx", "memory"
	);
	while(1);
}

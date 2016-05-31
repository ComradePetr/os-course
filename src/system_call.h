#ifndef __SYSTEM_CALL_H__
#define __SYSTEM_CALL_H__

#define SYSCALL_INTNO 48
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

struct args0_s{
	char* to_print;
};
struct args1_s{
	int to_print;
};
typedef struct args0_s args0;
typedef struct args1_s args1;

#endif /*__SYSTEM_CALL_H__*/

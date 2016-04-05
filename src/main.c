#include "kmem_cache.h"
#include "interrupt.h"
#include "memory.h"
#include "serial.h"
#include "paging.h"
#include "stdio.h"
#include "misc.h"
#include "time.h"
#include "threads.h"


#define THREADS_CNT 2
void f(void *args) {
    int arg =  *((int*)args);
    printf("Thread %d\n", arg);
    for(int i=0;i<10;++i){
        printf("%d: %d\n", arg, i);
        yield();
    }
}


void test_threads_simple() {
    int args[THREADS_CNT];
    thread_t* threads[THREADS_CNT];
    for (int i = 0; i < THREADS_CNT; i++) {
        args[i] = i;
        threads[i] = thread_create(&f, (void*) &args[i]);    
        printf("Created thread #%d\n", i);
    }

    for (int i = 0; i < THREADS_CNT; i++) 
        thread_start(threads[i]);
    for (int i = 0; i < THREADS_CNT; i++) {
        thread_join(threads[i], NULL);
        printf("Thread %d: joined\n", i);
    }
    puts("Test simple finished");
}


void main(void)
{
	setup_serial();
	setup_misc();
	setup_ints();
	setup_memory();
	setup_buddy();
	setup_paging();
	setup_alloc();
	setup_time();
	setup_threads();
	local_irq_enable();
	
	test_threads_simple();

	while (1);
}

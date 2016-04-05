#ifndef THREADS_H
#define THREADS_H

#define STACK_SIZE 8192

#include <sys/types.h>
#include <stdint.h>
#include "list.h"

void lock();
void unlock();

void init_threads();

typedef enum {NOT_STARTED, RUNNING, FINISHED, JOINABLE} thread_state;
struct thread {
	struct vlist_head link;

    char *rspl, *rsp;
    pid_t thread_id;
    uint64_t exit_code;
    thread_state state;
};

typedef volatile struct thread thread_t;


thread_t* thread_create(void (*fptr)(void *), void *arg);
void thread_start(thread_t *thread);
void yield();
void thread_exit(uint64_t code);
void setup_threads();
void thread_join(thread_t* thread, void **retval);
pid_t get_current_thread();
void scheduler(int irq);

void switch_threads(void **old_sp, void *new_sp);

#endif /* THREADS_H */

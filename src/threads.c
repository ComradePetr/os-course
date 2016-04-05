#include "threads.h"
#include "list.h"
#include "interrupt.h"
#include "kmem_cache.h"
#include "time.h"
#include<stdbool.h>
#include<stdio.h>


#define MAX_THREADS 65536
static thread_t *current_thread = NULL, *old_thread = NULL;
static thread_t threads_mem[MAX_THREADS];

static int locks_count = 0;
void lock(){
    local_irq_disable();
    ++locks_count;
}
void unlock(){
    if(!--locks_count)
		local_irq_enable();
}

static volatile VLIST_HEAD(threads);

void register_thread(thread_t *thread)
{ vlist_add_tail(&thread->link, &threads); }

void unregister_thread(thread_t *thread)
{ vlist_del(&thread->link); }

thread_t* get_thread(pid_t id)
{
	vlist_head_t * const end = &threads;
	vlist_head_t *pos = threads.next;

	for (; pos != end; pos = pos->next) {
		thread_t *thread = LIST_ENTRY(pos, struct thread, link);

		if(thread -> thread_id == id)
			return thread;
	}
	return NULL;
}

thread_t* next_thread(thread_t *thread)
{
	vlist_head_t *pos = thread->link.next;
	thread = LIST_ENTRY(pos, struct thread, link);

	while(thread -> state != RUNNING)
		pos = pos->next, thread = LIST_ENTRY(pos, struct thread, link);
	return thread;
}


void scheduler(int irq){
	static int counter=0;
	++counter;
	if (counter*HZ>=100){
		counter=0;
        unmask_irq(irq);
        yield();
	}
	unmask_irq(irq);
}

void setup_threads() {
	lock();
    thread_t *thread = &threads_mem[0];
    thread->thread_id = 0;
	thread->state = RUNNING;
	register_thread(thread);
	current_thread = thread;
	unlock();
}

static void thread_run(thread_t* new_thread) {
    if (current_thread != new_thread) {
		old_thread = current_thread;
        current_thread = new_thread;
        switch_threads((void**)&old_thread->rsp, (void*)current_thread->rsp);
    }
}

void thread_exit(uint64_t code) {
    local_irq_disable();
    current_thread->state = FINISHED;
    current_thread->exit_code = code;
    thread_t *nthread = next_thread(current_thread);
    unregister_thread(current_thread);
    thread_run(nthread);
    local_irq_enable();
}

void thread_exit0(){
    thread_exit(0);
}


thread_t* thread_create(void (*fptr) (void *), void *arg) {
    lock();
    static pid_t thread_id = 1;
    int id = thread_id;

    thread_t *thread = &threads_mem[id];

    thread->thread_id = id;
    thread->rspl = kmem_alloc(STACK_SIZE);
    thread->rsp = thread->rspl + STACK_SIZE;
    uint64_t* rsp = (uint64_t*) thread->rsp;
    *--rsp = (uint64_t) &thread_exit0;
    *--rsp = (uint64_t) fptr;
    *--rsp = (uint64_t) arg;
    *--rsp = 0; // rflags
    *--rsp = 0; // rbx
    *--rsp = 0; // rbp
    *--rsp = 0; // r12
    *--rsp = 0; // r13
    *--rsp = 0; // r14
    *--rsp = 0; // r15
    thread->rsp = (char*) rsp;
	thread->state = NOT_STARTED;
	++thread_id;
	
	unlock();

    return thread;
}

void thread_start(thread_t *thread){
	thread->state = RUNNING;
	register_thread(thread);
}

extern void check_finished() {
    if (old_thread && old_thread->state == FINISHED) {
        old_thread->state = JOINABLE;
        kmem_free((void*)old_thread->rspl);
    }
}


void yield() {
    local_irq_disable();
    thread_run(next_thread(current_thread));
    local_irq_enable();
}

void thread_join(thread_t *thread, void **retval) {
    while (thread -> state != JOINABLE)
        yield();

    local_irq_disable();
    unregister_thread(thread);
    if (retval)
        *retval = (void*) thread->exit_code;
    local_irq_enable();
}

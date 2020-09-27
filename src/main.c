#include <avr/io.h>
#include <util/delay.h>
#include <assert.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_THREADS 5

// Bytes needed to preserve current stack frame
#define FRAME_BACKUP_SIZE 21
#define THREAD_STACK_SIZE 0x100

typedef struct
{
	uint8_t *sp;
	uint8_t stack[THREAD_STACK_SIZE];
} Thread;

typedef struct
{
	Thread *threads[MAX_THREADS];
	uint8_t n_threads;
} Scheduler;

extern void scheduler_resume_thread(Thread *to);
extern void scheduler_switch_threads(Thread *from, Thread *to);

Scheduler *scheduler_inst;

void scheduler_init(Scheduler *scheduler)
{
	scheduler->n_threads = 0;
}

uint8_t scheduler_register_thread(Scheduler *scheduler, Thread *coroutine, void (*start)())
{
	// Init coroutine stack
	memset((void *)coroutine->stack, 0, THREAD_STACK_SIZE);
	coroutine->sp = coroutine->stack + THREAD_STACK_SIZE - FRAME_BACKUP_SIZE - 1;
	// Setup return address
	coroutine->stack[THREAD_STACK_SIZE - 2] = (uint16_t)start >> 8;
	coroutine->stack[THREAD_STACK_SIZE - 1] = (uint16_t)start & 0xFF;

	// Register with scheduler
	assert(scheduler->n_threads < MAX_THREADS);
	int id = scheduler->n_threads++;
	scheduler->threads[id] = coroutine;
	
	return id;
}

void test1()
{
	uint16_t x = 0;
	while (1)
	{
		x += 5;
		printf("test1 %x", x);
		scheduler_switch_threads(scheduler_inst->threads[0], scheduler_inst->threads[1]);
	}
}

void test2()
{
	uint16_t x = 0;	
	while (1)
	{
		x += 1;
		printf("test2 %x", x);
		scheduler_switch_threads(scheduler_inst->threads[1], scheduler_inst->threads[0]);
	}
}

int main(void)
{
	sei();

	Scheduler scheduler;
	scheduler_init(&scheduler);

	Thread t1;
	scheduler_register_thread(&scheduler, &t1, test1);

	Thread t2;
	scheduler_register_thread(&scheduler, &t2, test2);

	scheduler_inst = &scheduler;

	scheduler_resume_thread(&t1);
}
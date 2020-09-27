#include <avr/io.h>
#include <util/delay.h>
#include <assert.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


// Bytes needed to preserve current stack frame
#define FRAME_BACKUP_SIZE 21

#define MAX_THREADS 5
#define THREAD_STACK_SIZE 0x100
#define THREAD_ID_SCHEDULER 0
#define THREAD_ID_USER_START 1

typedef union
{
	uint8_t byte;
	uint16_t word;
} PipeData;

typedef struct
{
	char ready_bool;
	PipeData value;
} Pipe;

void pipe_init(Pipe *pipe)
{
	pipe->ready_bool = 0;
}

typedef struct
{
	uint8_t *sp;
	uint8_t stack[THREAD_STACK_SIZE];
} Thread;

void thread_init(Thread *thread)
{
	thread->sp = 0;
}

typedef struct
{
	Thread *threads[MAX_THREADS];
	Pipe *pipes[MAX_THREADS];
} Scheduler;

void scheduler_init(Scheduler *scheduler)
{
	for (uint8_t i = 0; i < MAX_THREADS; ++i)
	{
		scheduler->threads[i] = 0;
		scheduler->pipes[i] = 0;
	}
}

////////////////////////////////////////
// Scheduler

Scheduler *scheduler_inst = 0;
uint8_t current_thread = 0;

extern void _scheduler_resume_thread(Thread *to);
extern void _scheduler_switch_threads(Thread *from, Thread *to);

void scheduler_resume_thread(uint8_t thread_id)
{
	current_thread = thread_id;
	Thread *thread = scheduler_inst->threads[thread_id];
	_scheduler_resume_thread(thread);
}

void scheduler_switch_threads(uint8_t thread_id)
{
	Thread *starting_thread = scheduler_inst->threads[current_thread];
	Thread *ending_thread = scheduler_inst->threads[thread_id];
	current_thread = thread_id;
	_scheduler_switch_threads(starting_thread, ending_thread);
}

void scheduler_await(Pipe *pipe)
{
	assert(scheduler_inst->pipes[current_thread] == 0);
	scheduler_inst->pipes[current_thread] = pipe;
	scheduler_switch_threads(THREAD_ID_SCHEDULER);
}

void scheduler_set_global_inst(Scheduler *scheduler)
{
	scheduler_inst = scheduler;
	current_thread = THREAD_ID_SCHEDULER;
}

uint8_t scheduler_register_thread(Thread *thread, void (*start)())
{
	// Init thread stack
	memset((void *)thread->stack, 0, THREAD_STACK_SIZE);
	thread->sp = thread->stack + THREAD_STACK_SIZE - FRAME_BACKUP_SIZE - 1;
	// Setup return address
	thread->stack[THREAD_STACK_SIZE - 2] = (uint16_t)start >> 8;
	thread->stack[THREAD_STACK_SIZE - 1] = (uint16_t)start & 0xFF;

	// Register with scheduler
	uint8_t i;
	for (i = 0; i < MAX_THREADS; ++i)
	{
		if (scheduler_inst->threads[i] == 0)
		{
			scheduler_inst->threads[i] = thread;
			break;
		}
	}
	assert (i < MAX_THREADS);
	
	return i;
}

////////////////////////////////////////
// Timer

void timer_request(Pipe *pipe, uint16_t ms)
{
	// TODO: timer stuff
	pipe->ready_bool = 1;
}

void await_sleep(uint16_t ms)
{
	Pipe pipe;
	pipe_init(&pipe);
	timer_request(&pipe, ms);
	scheduler_await(&pipe);
}

////////////////////////////////////////
// Tasks/Threads

void scheduler_main()
{
	// Start all the threads
	for (uint8_t i = THREAD_ID_USER_START; i < MAX_THREADS; i++)
	{
		if (scheduler_inst->threads[i] != 0)
		{
			scheduler_switch_threads(i);
		}
	}

	while (1)
	{
		for (uint8_t i = THREAD_ID_USER_START; i < MAX_THREADS; i++)
		{
			Pipe *pipe = scheduler_inst->pipes[i];
			if (pipe != 0 && pipe->ready_bool)
			{
				scheduler_inst->pipes[i] = 0;
				scheduler_switch_threads(i);
			}
		}
	}

}

void task1_main()
{
	uint16_t x = 0;
	while (1)
	{
		x += 5;
		printf("task1 %x", x);
		await_sleep(500);
	}
}

void task2_main()
{
	uint16_t x = 0;
	while (1)
	{
		x += 1;
		printf("task2 %x", x);
		await_sleep(250);
	}
}

////////////////////////////////////////
// BOOT

int main(void)
{
	sei();

	Scheduler scheduler;
	scheduler_init(&scheduler);
	scheduler_set_global_inst(&scheduler);

	Thread sched_thread, thread_t1, thread_t2;
	thread_init(&sched_thread);
	thread_init(&thread_t1);
	thread_init(&thread_t2);

	uint8_t sched_id = scheduler_register_thread(&sched_thread, scheduler_main);
	assert(THREAD_ID_SCHEDULER == sched_id);

	scheduler_register_thread(&thread_t1, task1_main);
	scheduler_register_thread(&thread_t2, task2_main);

	scheduler_resume_thread(sched_id);
}
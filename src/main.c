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

#define MAX_TIMERS 5
#define CLOCK_TICKS_PER_SEC 122

typedef struct {
	Pipe *pipes[MAX_TIMERS];
	uint16_t countdown[MAX_TIMERS];
} TimerManager;

TimerManager *timer_manager_inst = 0;

void timer_manager_init(TimerManager *inst)
{
	for (uint8_t i = 0; i < MAX_TIMERS; ++i)
	{
		inst->pipes[i] = 0;
		inst->countdown[i] = 0;
	}
}

void timer_manager_set_global_inst(TimerManager *inst)
{
	timer_manager_inst = inst;
}

uint8_t max(uint8_t a, uint8_t b)
{
	if (a > b)
	{
		return a;
	}
	return b;
}

void timer_request(Pipe *pipe, uint16_t ms)
{
	uint8_t i;
	for (i = 0; i < MAX_TIMERS; i++)
	{
		if (timer_manager_inst->pipes[i] == 0)
		{
			timer_manager_inst->countdown[i] = max(ms >> 3, 1);  // approx conversion to clock ticks
			timer_manager_inst->pipes[i] = pipe;  // set this last in case ISR runs
			break;
		}
	}
	assert(i < MAX_TIMERS);
}

void await_sleep(uint16_t ms)
{
	Pipe pipe;
	pipe_init(&pipe);
	timer_request(&pipe, ms);
	scheduler_await(&pipe);
}

void timer_manager_init_hardware()
{
	// Use Timer1 overflow interrupt for sleeps
	TCCR1B |= (1 << CS10);  // Prescale of 1
	TIMSK1 |= (1 << TOIE1);  // Interrupt on overflow (16bit)
}

ISR (TIMER1_OVF_vect)
{
	for (uint8_t i = 0; i < MAX_TIMERS; ++i)
	{
		if (timer_manager_inst->pipes[i] != 0)
		{
			uint16_t count = --timer_manager_inst->countdown[i];
			if (count == 0)
			{
				timer_manager_inst->pipes[i]->ready_bool = 1;
				timer_manager_inst->pipes[i] = 0;
			}
		}
	}
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
	DDRC |= 1;
	PORTC &= ~1;

	while (1)
	{
		for (uint8_t i = 0; i < 4; i++)
		{
			PORTC ^= 1;
			await_sleep(75);
		}
		await_sleep(1000);
	}
}

void task2_main()
{
	// test max frequency counter
	DDRC |= 2;
	uint16_t x = 0;
	while (1)
	{
		x += 1;
		if (x % 122 == 0)
		{
			PORTC ^= 2;
			x = 0;
		}
		await_sleep(8);
	}
}

////////////////////////////////////////
// BOOT

int main(void)
{
	timer_manager_init_hardware();

	TimerManager timer_manager;
	timer_manager_init(&timer_manager);
	timer_manager_set_global_inst(&timer_manager);

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

	sei();

	scheduler_resume_thread(sched_id);
}
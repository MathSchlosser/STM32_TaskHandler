#include <stdio.h>
#include <stdint.h>
#include "main.h"


/*	Global variables */
uint32_t psp_of_tasks[MAX_TASKS] = {T1_STACK_START, T2_STACK_START, T3_STACK_START, T4_STACK_START}; /* Stores PSP of tasks */
uint32_t task_handlers[MAX_TASKS];
uint8_t current_task = 0; /* Task 1 is running */

/*	Function prototypes */
extern void initialise_monitor_handles(void);

void task1_handler(void);
void task2_handler(void);
void task3_handler(void);
void task4_handler(void);

__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_of_stack);
__attribute__((naked)) void switch_sp_to_psp(void);

void init_tasks_stack(void);
void enable_processor_faults(void);
void update_next_task(void);

void init_systic_timer(uint32_t tick_hz);

uint32_t get_psp_value(void);


/*	Code implementation */

int main(void)
{
	initialise_monitor_handles();

	enable_processor_faults();

	init_scheduler_stack(SCHED_STACK_START);

	task_handlers[0] = (uint32_t)task1_handler;
	task_handlers[1] = (uint32_t)task2_handler;
	task_handlers[2] = (uint32_t)task3_handler;
	task_handlers[3] = (uint32_t)task4_handler;

	init_tasks_stack();

	init_systic_timer(TICK_HZ);

	switch_sp_to_psp();

	task1_handler();

	for(;;);
}

void task1_handler(void)
{
	while(1){
		printf("This is task 1.\n");
	}
}
void task2_handler(void)
{
	while(1){
		printf("This is task 2.\n");
	}
}
void task3_handler(void)
{
	while(1){
		printf("This is task 3.\n");
	}
}
void task4_handler(void)
{
	while(1){
		printf("This is task 4.\n");
	}
}

void init_systic_timer(uint32_t tick_hz)
{
	uint32_t *pSRVR = (uint32_t*)0xE000E014; /* Pointer to SysTick Reload Value Register */
	uint32_t *pSCSR = (uint32_t*)0xE000E010; /*	Pointer to SysTick Control and Status Register */

	uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz) - 1; /* n-1 because the exception will be triggered when the current value register reload the value in SRVR */

	/*	Clear the value in SVR */
	*pSRVR &= ~(0x00FFFFFF);

	/*	Load the value in SVR */
	*pSRVR |= count_value;

	/*	Settings */
	*pSCSR |= (1 << 2); /*	Clock source -> internal */
	*pSCSR |= (1 << 1); /*	Enable systick exception request */

	/*	Enable systick */
	*pSCSR |= (1 << 0); /*	Enable systick */
}

__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_of_stack)
{
	__asm volatile ("MSR MSP, %0"::"r"(sched_top_of_stack):);
	__asm volatile ("BX LR");
}



void init_tasks_stack(void)
{
	uint32_t *pPSP;

	for(int i = 0; i < MAX_TASKS; i++ )
	{
		pPSP = (uint32_t*)psp_of_tasks[i];

		pPSP--; /* Decrements because in ARM-M stack is full descendant */
		*pPSP = DUMMY_XPSR;

		pPSP--; /* PC */
		*pPSP = task_handlers[i];

		pPSP--; /* LR */
		*pPSP = 0xFFFFFFFD;

		for(int j = 0; j < 13; j++)
		{
			pPSP--;
			*pPSP = 0;
		}

		psp_of_tasks[i] = (uint32_t)pPSP;

	}

}

void enable_processor_faults(void)
{
	uint32_t *pSHCSR = (uint32_t*)0xE000ED24;

	*pSHCSR |= ( 1 << 16 ); /* Memory manage */
	*pSHCSR |= ( 1 << 17 ); /* Bus fault */
	*pSHCSR |= ( 1 << 18 ); /* Usage fault */

}


uint32_t get_psp_value(void)
{

	return psp_of_tasks[current_task];
}

void save_psp_value(uint32_t current_psp_value)
{
	psp_of_tasks[current_task] = current_psp_value;
}

void update_next_task(void)
{
	current_task++;
	current_task = current_task % MAX_TASKS;
}

__attribute__((naked)) void switch_sp_to_psp(void)
{
	/*	Initialize the PSP with TASK1 stack start address */
	__asm volatile ("PUSH {LR}"); /* Preserve LR connecting back to main function */
	__asm volatile ("BL get_psp_value"); /* Get PSP from get_psp_value function */
	__asm volatile ("MSR PSP, R0"); /* Initialize PSP */
	__asm volatile ("POP {LR}"); /* Retrieve LR value which was saved in PUSH instruction */

	/*	Change SP to PSP using control register */
	__asm volatile ("MOV R0, #0x02");
	__asm volatile ("MSR CONTROL, R0");
	__asm volatile ("BX LR");
}

__attribute__((naked)) void SysTick_Handler (void)
{
	/*	Save the context of current task */
	__asm volatile ("MRS R0, PSP");

	/*	Store multiple registers and decrement the pointer STMDB */
	__asm volatile ("STMDB R0!, {R4 - R11}");/*	Using PSP value, store in SF2 Registers R4 - R11 */
	__asm volatile ("PUSH {LR}");

	__asm volatile ("BL save_psp_value");/*	Save the current value of PSP */

	/*	Retrieve the context of current task */
	__asm volatile ("BL update_next_task");/* Decide next task to run */
	__asm volatile ("BL get_psp_value");/* Gets PSP past value */

	/*	Load multiple registers and increment the pointer LDMIA */
	__asm volatile ("LDMIA R0!, {R4-R11} ");/* Using PSP value, retrieve from SF2 Registers R4 - R11 */
	__asm volatile ("MSR PSP, R0"); /* Update PSP and exit */

	__asm volatile ("POP {LR}");

	__asm volatile ("BX LR");
}

void HardFault_Handler(void)
{
	printf("Exception  :  HardFault.\n");
	while(1);
}

void MemManage_Handler(void)
{
	printf("Exception  :  MemManage.\n");
	while(1);
}

void BusFault_Handler(void)
{
	printf("Exception  :  BusFault.\n");
	while(1);
}

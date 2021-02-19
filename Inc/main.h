/*
 * main.h
 *
 *  Created on: Feb 17, 2021
 *      Author: Matheus
 */

#ifndef MAIN_H_
#define MAIN_H_

/*	Macro definition */
#define MAX_TASKS				(4U)
#define DUMMY_XPSR			(0x01000000U)

/* Some stack memory calculation */
#define SIZE_TASK_STACK			(1024U)
#define SIZE_SCHED_STACK		(1024U)

#define SRAM_START				(0X20000000U)
#define SIZE_SRAM				((16) * (1024))
#define SRAM_END				((SRAM_START) + (SIZE_SRAM))

/*		MEMORY SCHEME TO FOLLOWING TASK SCHEDULER
 * 		0X20000000--------------------------------------------------------------16KB------------------------------------------------------------------------------RAM END
 * 		|--------- MEMORY -----------| PRIVATE TASK SCHEDULER | PRIVATE STACK T4 (1KB) | PRIVATE STACK T3 (1KB) | PRIVATE STACK T2 (1KB) | PRIVATE STACK T1 (1KB) |
 */

#define T1_STACK_START			(SRAM_END)
#define T2_STACK_START			((SRAM_END) - (1 * SIZE_TASK_STACK))
#define T3_STACK_START			((SRAM_END) - (2 * SIZE_TASK_STACK))
#define T4_STACK_START			((SRAM_END) - (3 * SIZE_TASK_STACK))
#define SCHED_STACK_START		((SRAM_END) - (4 * SIZE_TASK_STACK))

#define TICK_HZ					(1000U)

#define HSI_CLOCK				(8000000U)
#define SYSTICK_TIM_CLK			HSI_CLOCK


#endif /* MAIN_H_ */

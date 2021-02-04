/**
 ******************************************************************************
 * @file    main.c
 * @author  P. Foubet
 * @version V1.0
 * @date    31-December-2018
 * @brief   Default main function.
 ******************************************************************************
 */

#include "FreeRTOS.h"
#include "task.h"
#include "interface.h"
#define DELAY_START_TIME_TICKS 2000

typedef struct task_t {
	char name[20];
	uint16_t start;
	uint16_t period;
	uint16_t cpu;
	uint16_t priority;
	int led;
} task;

task tasks[] = { { "Task1", 0, 1000, 50, 1, LED3}, { "Task2", 1000, 2000, 500, 2, LED4}};

uint16_t cal = 1000;

void burnCPU(uint16_t ms) {
	while (ms--) {
		for (uint16_t i = 0; i < cal; i++) {
			__asm("nop");
		}
	}
}

void vTaskCalibrateBurnCPU(void* p) {
	BSP_LED_On(LED3);
	BSP_LED_On(LED4);
	TickType_t reference_ticks = 1000/portTICK_RATE_MS;
	TickType_t mesured_ticks = xTaskGetTickCount();
	burnCPU(reference_ticks);
	mesured_ticks = xTaskGetTickCount() - mesured_ticks;
	cal = cal * reference_ticks / mesured_ticks;
	BSP_LED_Off(LED3);
	BSP_LED_Off(LED4);
	vTaskDelete(NULL);
}


void vTask(void* p) {
	uint16_t start = ((task*) p)->start/portTICK_RATE_MS;
	uint16_t period = ((task*) p)->period/portTICK_RATE_MS;
	uint16_t cpu = ((task*) p)->cpu/portTICK_RATE_MS;
	int led = ((task*) p)->led;

	// Wait first execution
	TickType_t xlastWakeTime = DELAY_START_TIME_TICKS + start;
	while (xTaskGetTickCount() < xlastWakeTime) vTaskDelay(1);

	while (1) {
		// Wait for the next cycle.
		vTaskDelayUntil(&xlastWakeTime, period);
		BSP_LED_On(led);
		burnCPU(cpu);
		BSP_LED_Off(led);
	}
}


int main(void) {
	interface_init();

	/* creation des threads */
	if (!(pdPASS == xTaskCreate(vTaskCalibrateBurnCPU, "Calibration", 64, NULL, configMAX_PRIORITIES-1, NULL))) goto err;
	if (!(pdPASS == xTaskCreate(vTask, tasks[0].name, 64, (void*)&tasks[0], configMAX_PRIORITIES-1-tasks[0].priority, NULL))) goto err;
	if (!(pdPASS == xTaskCreate(vTask, tasks[1].name, 64, (void*)&tasks[1], configMAX_PRIORITIES-1-tasks[1].priority, NULL))) goto err;

	/* lancement du systeme */
	vTaskStartScheduler();
	err:              // en principe jamais atteint !
	while (1);
	return 0;
}

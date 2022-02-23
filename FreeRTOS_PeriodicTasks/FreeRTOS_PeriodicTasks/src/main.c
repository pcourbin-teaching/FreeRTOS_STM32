/**
 ******************************************************************************
 * @file    main.c
 * @author  P. Courbin
 * @version V1.0
 * @date    04-February-2021
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
	uint8_t led3;
	uint8_t led4;
} task;
				//	Name		Start	Period	CPU		Prio	LED3				LED4
task tasks[] = { { "Task1", 	0, 		1000, 	400, 	1, 		GPIO_PIN_SET, 		GPIO_PIN_RESET 	},
				 { "Task2", 	1000, 	3000, 	700, 	2, 		GPIO_PIN_RESET, 	GPIO_PIN_SET 	}
				};

uint16_t cal = 1000;

void update_leds(uint8_t led3, uint8_t led4) {
	HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, led3);
	HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_PIN, led4);
}

void burnCPU(uint16_t ms, uint8_t led3, uint8_t led4) {
	while (ms--) {
		for (uint16_t i = 0; i < cal; i++) {
			update_leds(led3, led4);
			__asm("nop");
		}
	}
}

void vTaskCalibrateBurnCPU(void* p) {
	update_leds(GPIO_PIN_SET, GPIO_PIN_SET);
	TickType_t reference_ticks = 1000/portTICK_RATE_MS;
	TickType_t mesured_ticks = xTaskGetTickCount();
	burnCPU(reference_ticks, GPIO_PIN_SET, GPIO_PIN_SET);
	mesured_ticks = xTaskGetTickCount() - mesured_ticks;
	cal = cal * reference_ticks / mesured_ticks;
	update_leds(GPIO_PIN_RESET, GPIO_PIN_RESET);
	vTaskDelete(NULL);
}

void vTask(void* p) {
	uint16_t start = ((task*) p)->start/portTICK_RATE_MS;
	uint16_t period = ((task*) p)->period/portTICK_RATE_MS;
	uint16_t cpu = ((task*) p)->cpu/portTICK_RATE_MS;
	uint8_t led3 = ((task*) p)->led3;
	uint8_t led4 = ((task*) p)->led4;

	// Wait and do first execution
	TickType_t xlastWakeTime = 0;
	vTaskDelayUntil(&xlastWakeTime, DELAY_START_TIME_TICKS/portTICK_RATE_MS + start);
	burnCPU(cpu, led3, led4);
	update_leds(GPIO_PIN_RESET, GPIO_PIN_RESET);

	while (1) {
		// Wait for the next cycle.
		vTaskDelayUntil(&xlastWakeTime, period);
		burnCPU(cpu, led3, led4);
		update_leds(GPIO_PIN_RESET, GPIO_PIN_RESET);
	}
}

int main(void) {
	interface_init();
	uint8_t i, nb_task = sizeof(tasks)/sizeof(tasks[0]);

	/* creation des threads */
	if (!(pdPASS == xTaskCreate(vTaskCalibrateBurnCPU, "Calibration", 64, NULL, configMAX_PRIORITIES-1, NULL))) goto err;
	for (i = 0; i < nb_task; i++) {
		if (!(pdPASS == xTaskCreate(vTask, tasks[i].name, 64, (void* )&tasks[i], configMAX_PRIORITIES-1-tasks[i].priority, NULL))) goto err;
	}
	/* lancement du systeme */
	vTaskStartScheduler();
err:              // en principe jamais atteint !
	while (1);
	return 0;
}

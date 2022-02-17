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
#include "semphr.h"
#include "time.h"
#include "unistd.h"

int buttonPushed = 0;
SemaphoreHandle_t mutexLEDs = NULL;

void update_leds(uint8_t led3, uint8_t led4) {
	if( xSemaphoreTake( mutexLEDs, portMAX_DELAY ) == pdTRUE ){
		HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, led3);
		HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_PIN, led4);
		xSemaphoreGive( mutexLEDs );
	}
}

void vLedsVerte(void* p)
{
	int i, nbIter = 100000;
	while(1){
		for(i=0; i<nbIter; i++){
			update_leds(1,0);
		}
		update_leds(0,0);
		vTaskDelay(1000/portTICK_RATE_MS);
	}
}

void vLedsRouge(void* p)
{
	int i, nbIter = 200000;
	while(1){
		for(i=0; i<nbIter; i++){
			update_leds(0,1);
		}
		update_leds(0,0);
		vTaskDelay(2000/portTICK_RATE_MS);
	}
}

void buttonCheck(void* p)
{
	int i, nbIter = 500000;
	while(1){
		if(buttonPushed==1){
			buttonPushed = 0;
			for(i=0; i<nbIter; i++){
				update_leds(1,1);
			}
			update_leds(0,0);
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == KEY_BUTTON_PIN){
		buttonPushed = 1;
	}
}

int main(void)
{
	char *Nom1="LedVerte", *Nom2="LedRouge", *Nom3="ButtonCheck";
	interface_init();
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
	mutexLEDs = xSemaphoreCreateMutex();

	/* creation des threads */
	if (!(pdPASS == xTaskCreate( vLedsVerte, Nom1, 64, NULL, 3, NULL ))) goto err;
	if (!(pdPASS == xTaskCreate( vLedsRouge, Nom2, 64, NULL, 2, NULL ))) goto err;
	if (!(pdPASS == xTaskCreate( buttonCheck, Nom3, 64, NULL, 1, NULL ))) goto err;

	/* lancement du systeme */
	vTaskStartScheduler();
err:              // en principe jamais atteint !
	while(1);
	return 0;
}

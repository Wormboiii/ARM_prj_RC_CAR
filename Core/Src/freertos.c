/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint8_t rxData[2];
uint8_t btrxData[2];
int fwState;
int bwState;
int footBreak;
int gearShift;
int handleState;
int gearSignal;

uint16_t dutyRate = 500;
/* USER CODE END Variables */
/* Definitions for MOTOR_CONTROL */
osThreadId_t MOTOR_CONTROLHandle;
const osThreadAttr_t MOTOR_CONTROL_attributes = {
  .name = "MOTOR_CONTROL",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UART */
osThreadId_t UARTHandle;
const osThreadAttr_t UART_attributes = {
  .name = "UART",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for gearTask */
osThreadId_t gearTaskHandle;
const osThreadAttr_t gearTask_attributes = {
  .name = "gearTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	HAL_UART_Receive_DMA(&huart6, btrxData, 1);

	// 블투
	if(btrxData[0] == 'w') {

		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
			TIM4->CCR1 = dutyRate;
			TIM4->CCR2 = dutyRate;
	}
	else if(btrxData[0] == 's') {

		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
			TIM4->CCR1 = dutyRate;
			TIM4->CCR2 = dutyRate;
	}

	else if(btrxData[0] == 'a') {
		TIM4->CCR1 = 0;
		TIM4->CCR2 = dutyRate;
	}
	else if(btrxData[0] == 'd') {
		TIM4->CCR1 = dutyRate;
		TIM4->CCR2 = 0;
	}

//	else if(btrxData[0] == 'x') {
//		footBreak = 1;
//		gearShift = 0;
//	}
//	else if(btrxData[0] == 'i') {
//		gearShift = 0;
//		fwState = 0;
//		bwState = 0;
//	}


	if(btrxData[0] == 'l') {
		gearSignal = 1;
	}
	else if(btrxData[0] == 'p') {
		gearSignal = 2;
	}

}
/* USER CODE END FunctionPrototypes */

void START_MOTOR_CONTROL(void *argument);
void START_UART_TASK(void *argument);
void Start_gearTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	HAL_UART_Receive_DMA(&huart2, rxData, 1);
	HAL_UART_Receive_DMA(&huart6, btrxData, 1);
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of MOTOR_CONTROL */
  MOTOR_CONTROLHandle = osThreadNew(START_MOTOR_CONTROL, NULL, &MOTOR_CONTROL_attributes);

  /* creation of UART */
  UARTHandle = osThreadNew(START_UART_TASK, NULL, &UART_attributes);

  /* creation of gearTask */
  gearTaskHandle = osThreadNew(Start_gearTask, NULL, &gearTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_START_MOTOR_CONTROL */
/**
  * @brief  Function implementing the MOTOR_CONTROL thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_START_MOTOR_CONTROL */
void START_MOTOR_CONTROL(void *argument)
{
  /* USER CODE BEGIN START_MOTOR_CONTROL */
  /* Infinite loop */
	for(;;)
	{
		if(gearShift == 0) {
			TIM4->CCR1 = 0;
			TIM4->CCR2 = 0;
			dutyRate = 500;
		}
		else if(gearShift == 1) {
			TIM4->CCR1 = 0;
			TIM4->CCR2 = 0;
			dutyRate = 700;
		}
		else if(gearShift == 2) {
			TIM4->CCR1 = 0;
			TIM4->CCR2 = 0;
			dutyRate = 900;
		}
		if(footBreak == 1) {
			TIM4->CCR1 = 0;
			TIM4->CCR2 = 0;
		}
    osDelay(10);
  }
  /* USER CODE END START_MOTOR_CONTROL */
}

/* USER CODE BEGIN Header_START_UART_TASK */
/**
* @brief Function implementing the UART thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_START_UART_TASK */
void START_UART_TASK(void *argument)
{
  /* USER CODE BEGIN START_UART_TASK */
  /* Infinite loop */
  for(;;)
  {
	  if(fwState == 1) {
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	  }
	  else if(bwState == 1) {
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	  }
	  osDelay(10);
  }
  /* USER CODE END START_UART_TASK */
}

/* USER CODE BEGIN Header_Start_gearTask */
/**
* @brief Function implementing the gearTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_gearTask */
void Start_gearTask(void *argument)
{
  /* USER CODE BEGIN Start_gearTask */
  /* Infinite loop */
  for(;;)
  {
		if(gearSignal == 1) {
			if(gearShift > 0) {
				gearShift -= 1;
			}
			HAL_Delay(100);
			gearSignal = 0;
		}
		else if(gearSignal == 2) {
			if(gearShift < 2) {
				gearShift += 1;
			}
			HAL_Delay(100);
			gearSignal = 0;
		}
    osDelay(10);
  }
  /* USER CODE END Start_gearTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


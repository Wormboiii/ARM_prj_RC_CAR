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
#include "string.h"
#include "motor.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;

extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim9;
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
uint8_t honkState;
uint8_t tlSignal;
uint8_t trSignal;
uint8_t brkSignal;

int fwState;
int bwState;
int footBreak;
int handleState;
int gearSignal;

volatile int gearShift;

volatile uint8_t tx_busy = 0;

char gear0[] = "z";
char gear1[] = "o";
char gear2[] = "t";
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

	if(btrxData[0] == 'b') {
		allStop();
		brkSignal = 0xff;
	}

	else if(btrxData[0] == 'w') {
		moveForward();
		tlSignal = 0x00;
		trSignal = 0x00;
		brkSignal = 0x00;
	}
	else if(btrxData[0] == 's') {
		moveBackward();
		tlSignal = 0x00;
		trSignal = 0x00;
		brkSignal = 0x00;
	}
	else if(btrxData[0] == 'a') {
		turnLeft();
		brkSignal = 0x00;
	}
	else if(btrxData[0] == 'd') {
		turnRight();
		brkSignal = 0x00;
	}



	if(btrxData[0] == 'l') {
		gearSignal = 1;
	}
	else if(btrxData[0] == 'p') {
		gearSignal = 2;
	}


	if(btrxData[0] == 'h') {
		honkState = ~honkState;
	}


	if(btrxData[0] == 'q') {
		tlSignal = ~tlSignal;
	}
	else if(btrxData[0] == 'e') {
		trSignal = ~trSignal;
	}

}



void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART6) {
        tx_busy = 0;
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
	HAL_TIM_Base_Start(&htim5);
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
	int prevGearShift = -1;
  /* Infinite loop */
	for(;;)
	{
		if(gearShift == 0) {
			gearZero();
		}
		else if(gearShift == 1) {
			gearFirst();
		}
		else if(gearShift == 2) {
			gearSecond();
		}
		if(footBreak == 1) {
			allStop();
		}


		if(gearShift != prevGearShift && tx_busy == 0) {
			prevGearShift = gearShift;
			tx_busy = 1;
			if(gearShift == 0) {
				HAL_UART_Transmit_DMA(&huart6, (uint8_t *)gear0, strlen(gear0));
			}
			else if(gearShift == 1) {
				HAL_UART_Transmit_DMA(&huart6, (uint8_t *)gear1, strlen(gear1));
			}
			else if(gearShift == 2) {
				HAL_UART_Transmit_DMA(&huart6, (uint8_t *)gear2, strlen(gear2));
			}
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
	  if(honkState == 0x00) {
		  TIM11->CCR1 = 0;
	  }
	  else if(honkState == 0xff) {
		  TIM11->CCR1 = 194;
	  }

	  if(tlSignal == 0x00) {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
		  HAL_TIM_Base_Stop_IT(&htim5);
	  }
	  else if(tlSignal == 0xff) {
		  HAL_TIM_Base_Start_IT(&htim5);
	  }

	  if(trSignal == 0x00) {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
		  HAL_TIM_Base_Stop_IT(&htim9);
	  }
	  else if(trSignal == 0xff) {
		  HAL_TIM_Base_Start_IT(&htim9);
	  }

	  if(brkSignal == 0xff) {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
	  }
	  else if(brkSignal == 0x00) {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
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
			HAL_Delay(200);
			gearSignal = 0;
		}
		else if(gearSignal == 2) {
			if(gearShift < 2) {
				gearShift += 1;
			}
			HAL_Delay(200);
			gearSignal = 0;
		}
    osDelay(10);
  }
  /* USER CODE END Start_gearTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


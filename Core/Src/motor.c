/*
 * motor.c
 *
 *  Created on: Jul 8, 2025
 *      Author: wonhyeok
 */

#include "motor.h"

uint16_t dutyRate = 500;

void moveForward() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	TIM4->CCR1 = dutyRate;
	TIM4->CCR2 = dutyRate;
}

void moveBackward() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	TIM4->CCR1 = dutyRate;
	TIM4->CCR2 = dutyRate;
}

void turnLeft() {
	TIM4->CCR1 = dutyRate;
	TIM4->CCR2 = 0;
}

void turnRight() {
	TIM4->CCR1 = 0;
	TIM4->CCR2 = dutyRate;
}

void gearZero() {
	TIM4->CCR1 = 0;
	TIM4->CCR2 = 0;
	dutyRate = 500;
}

void gearFirst() {
	TIM4->CCR1 = 0;
	TIM4->CCR2 = 0;
	dutyRate = 700;
}

void gearSecond() {
	TIM4->CCR1 = 0;
	TIM4->CCR2 = 0;
	dutyRate = 900;
}

void allStop() {
	TIM4->CCR1 = 0;
	TIM4->CCR2 = 0;
}













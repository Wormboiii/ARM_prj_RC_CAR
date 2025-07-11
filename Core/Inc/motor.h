/*
 * motor.h
 *
 *  Created on: Jul 8, 2025
 *      Author: wonhyeok
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "main.h"

// 모터 컨트롤 1~4번핀 아웃풋 설정해서 방향 제어.
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();

// PWM 듀티비 제어해서 속도 제어.
void gearZero();
void gearFirst();
void gearSecond();
void allStop();




#endif /* INC_MOTOR_H_ */

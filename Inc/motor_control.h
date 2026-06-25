/******************************************************************************
 * @file    motor_control.h
 * @author  Morel
 * @brief   Public interface for the application-level motor control module.
 ******************************************************************************/

#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

#include <stdint.h>

/**
 * @brief Available predefined motor operating modes.
 */
typedef enum
{
    MODE_ECO = 0,
    MODE_NORMAL,
    MODE_FAST,
    MODE_COUNT
} MotorMode_t;

/**
 * @brief Motor rotation direction.
 */
typedef enum
{
    DIR_FORWARD = 0,
    DIR_BACKWARD,
    DIR_COUNT
} MotorDirection_t;

/**
 * @brief High-level motor state.
 */
typedef enum
{
    MOTOR_IDLE = 0,
    MOTOR_RUNNING,
    MOTOR_STOPPED,
    MOTOR_ERROR
} MotorState_t;

/**
 * @brief Motor profile configuration.
 */
typedef struct
{
    uint16_t steps;
    uint32_t stepDelayMs;
} MotorProfile_t;

void MotorControl_init(void);
void MotorControl_Stop(void);

void MotorControl_RunUserMode(void);
void MotorControl_RunAdminCustom(void);
void MotorControl_Task(void);

void MotorControl_SetMode(MotorMode_t mode);
void MotorControl_SetDirection(MotorDirection_t dir);

void MotorControl_SetAdminSteps(uint16_t steps);
void MotorControl_SetAdminDelay(uint32_t delayMs);

MotorMode_t MotorControl_GetMode(void);
MotorDirection_t MotorControl_GetDirection(void);
MotorState_t MotorControl_GetState(void);

uint16_t MotorControl_GetAdminSteps(void);
uint32_t MotorControl_GetAdminDelay(void);

const char* MotorControl_GetModeString(void);
const char* MotorControl_GetStateString(void);
const char* MotorControl_GetDirectionString(void);

#endif /* MOTOR_CONTROL_H_ */

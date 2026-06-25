/******************************************************************************
 * @file    motor_control.c
 * @author  Morel
 * @brief   Application-level motor control state machine.
 *
 * This module implements the high-level motor control logic. It selects motor
 * profiles, manages direction changes, handles pause intervals and calls the
 * low-level stepper driver to generate individual motor steps.
 *
 * The module is designed to be called periodically from the main loop through
 * MotorControl_Task().
 ******************************************************************************/

#include <motor_control.h>
#include <stepper.h>
#include <timer.h>
#include <stdbool.h>

#define MOTOR_DEFAULT_STEP_DELAY_MS     1U
#define MOTOR_DEFAULT_PAUSE_TIME_MS     1000U

static uint32_t lastStepTime_ms = 0U;
static uint32_t stepDelay_ms = MOTOR_DEFAULT_STEP_DELAY_MS;

static uint32_t targetSteps = 0U;
static uint32_t currentSteps = 0U;

static uint32_t pauseStartTime_ms = 0U;
static uint32_t pauseTime_ms = MOTOR_DEFAULT_PAUSE_TIME_MS;
static bool pauseActive = false;

static MotorMode_t currentMode = MODE_NORMAL;
static MotorDirection_t currentDirection = DIR_FORWARD;
static MotorState_t currentState = MOTOR_IDLE;

static uint16_t adminSteps = 0U;
static uint32_t adminDelayMs = MOTOR_DEFAULT_STEP_DELAY_MS;

/*
 * User motor profiles.
 *
 * Each profile defines the number of half-steps and the delay between steps.
 * For a 28BYJ-48 motor in half-step mode, 4096 half-steps are commonly used
 * for approximately one output shaft revolution.
 */
static const MotorProfile_t userProfiles[] =
{
    [MODE_ECO]    = {1024U, MOTOR_DEFAULT_STEP_DELAY_MS},
    [MODE_NORMAL] = {2048U, MOTOR_DEFAULT_STEP_DELAY_MS},
    [MODE_FAST]   = {4096U, MOTOR_DEFAULT_STEP_DELAY_MS}
};

/******************************************************************************
 * @brief Initializes the motor control module.
 ******************************************************************************/
void MotorControl_init(void)
{
    stepper_init();

    currentMode = MODE_NORMAL;
    currentDirection = DIR_FORWARD;
    currentState = MOTOR_IDLE;

    pauseActive = false;
    currentSteps = 0U;
    targetSteps = 0U;
}

/******************************************************************************
 * @brief Stops the motor and disables all stepper coils.
 ******************************************************************************/
void MotorControl_Stop(void)
{
    stepper_stop();

    pauseActive = false;
    currentState = MOTOR_STOPPED;
}

/******************************************************************************
 * @brief Starts the motor using the currently selected user profile.
 *
 * The selected profile defines the number of steps and the step delay. The
 * motor then runs until the target step count is reached.
 ******************************************************************************/
void MotorControl_RunUserMode(void)
{
    MotorProfile_t profile = userProfiles[currentMode];

    stepDelay_ms = profile.stepDelayMs;
    lastStepTime_ms = timer_ms();

    targetSteps = profile.steps;
    currentSteps = 0U;

    pauseActive = false;
    currentState = MOTOR_RUNNING;
}

/******************************************************************************
 * @brief Periodic motor control task.
 *
 * This function implements a non-blocking state machine. It must be called
 * periodically from the main loop. Step timing is based on timer_ms() and does
 * not block the CPU with delay loops.
 *
 * Behavior:
 *  - Generates one step after each elapsed step delay.
 *  - Counts the completed steps.
 *  - Stops the motor after reaching the target.
 *  - Waits for a pause interval.
 *  - Reverses direction and starts the next cycle.
 ******************************************************************************/
void MotorControl_Task(void)
{
    uint32_t now = timer_ms();

    if (currentState != MOTOR_RUNNING)
    {
        return;
    }

    if (pauseActive == true)
    {
        if ((now - pauseStartTime_ms) >= pauseTime_ms)
        {
            pauseActive = false;
            currentSteps = 0U;
            lastStepTime_ms = now;

            if (currentDirection == DIR_FORWARD)
            {
                currentDirection = DIR_BACKWARD;
            }
            else
            {
                currentDirection = DIR_FORWARD;
            }
        }

        return;
    }

    if ((now - lastStepTime_ms) >= stepDelay_ms)
    {
        lastStepTime_ms = now;

        if (currentDirection == DIR_FORWARD)
        {
            stepper_step_forward();
        }
        else
        {
            stepper_step_backward();
        }

        currentSteps++;

        if (currentSteps >= targetSteps)
        {
            stepper_stop();
            pauseStartTime_ms = now;
            pauseActive = true;
        }
    }
}

/******************************************************************************
 * @brief Sets the active motor profile.
 *
 * @param mode Motor mode to select.
 ******************************************************************************/
void MotorControl_SetMode(MotorMode_t mode)
{
    if (mode < MODE_COUNT)
    {
        currentMode = mode;
    }
}

/******************************************************************************
 * @brief Sets the active motor direction.
 *
 * @param dir Motor direction to select.
 ******************************************************************************/
void MotorControl_SetDirection(MotorDirection_t dir)
{
    if (dir < DIR_COUNT)
    {
        currentDirection = dir;
    }
}

/******************************************************************************
 * @brief Sets custom step count for admin mode.
 *
 * @param steps Number of half-steps.
 ******************************************************************************/
void MotorControl_SetAdminSteps(uint16_t steps)
{
    adminSteps = steps;
}

/******************************************************************************
 * @brief Sets custom step delay for admin mode.
 *
 * @param delayMs Delay between motor steps in milliseconds.
 ******************************************************************************/
void MotorControl_SetAdminDelay(uint32_t delayMs)
{
    if (delayMs >= 1U)
    {
        adminDelayMs = delayMs;
    }
}

/******************************************************************************
 * @brief Returns the currently selected motor mode.
 *
 * @return Current motor mode.
 ******************************************************************************/
MotorMode_t MotorControl_GetMode(void)
{
    return currentMode;
}

/******************************************************************************
 * @brief Returns the currently selected motor direction.
 *
 * @return Current motor direction.
 ******************************************************************************/
MotorDirection_t MotorControl_GetDirection(void)
{
    return currentDirection;
}

/******************************************************************************
 * @brief Returns the current motor state.
 *
 * @return Current motor state.
 ******************************************************************************/
MotorState_t MotorControl_GetState(void)
{
    return currentState;
}

/******************************************************************************
 * @brief Returns configured admin step count.
 *
 * @return Admin step count.
 ******************************************************************************/
uint16_t MotorControl_GetAdminSteps(void)
{
    return adminSteps;
}

/******************************************************************************
 * @brief Returns configured admin step delay.
 *
 * @return Admin step delay in milliseconds.
 ******************************************************************************/
uint32_t MotorControl_GetAdminDelay(void)
{
    return adminDelayMs;
}

/******************************************************************************
 * @brief Converts the current motor mode to a string.
 *
 * @return Pointer to constant mode string.
 ******************************************************************************/
const char* MotorControl_GetModeString(void)
{
    switch (MotorControl_GetMode())
    {
        case MODE_ECO:    return "ECO";
        case MODE_NORMAL: return "NORMAL";
        case MODE_FAST:   return "FAST";
        default:          return "UNKNOWN";
    }
}

/******************************************************************************
 * @brief Converts the current motor state to a string.
 *
 * @return Pointer to constant state string.
 ******************************************************************************/
const char* MotorControl_GetStateString(void)
{
    switch (MotorControl_GetState())
    {
        case MOTOR_IDLE:    return "IDLE";
        case MOTOR_RUNNING: return "RUNNING";
        case MOTOR_STOPPED: return "STOPPED";
        case MOTOR_ERROR:   return "ERROR";
        default:            return "UNKNOWN";
    }
}

/******************************************************************************
 * @brief Converts the current motor direction to a string.
 *
 * @return Pointer to constant direction string.
 ******************************************************************************/
const char* MotorControl_GetDirectionString(void)
{
    switch (MotorControl_GetDirection())
    {
        case DIR_FORWARD:  return "FORWARD";
        case DIR_BACKWARD: return "BACKWARD";
        default:           return "UNKNOWN";
    }
}

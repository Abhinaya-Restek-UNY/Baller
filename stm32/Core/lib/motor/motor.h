
#ifndef __MOTOR_H
#define __MOTOR_H

#define PWM_RESOLUTION 8192
#include "main.h"
#include <math.h>

typedef int16_t motor_direction_t;
// Motor context
typedef struct {
  TIM_HandleTypeDef *htim;
  uint32_t channel;

  GPIO_TypeDef *GPIOx_in1;
  uint16_t pin_in1;

  GPIO_TypeDef *GPIOx_in2;
  uint16_t pin_in2;
} motor_handle_t;

void motor_init(motor_handle_t *context);
void motor_set_direction(motor_handle_t *context, motor_direction_t direction);

#endif

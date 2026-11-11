/* Modelo host: conserva tipos originales y redirige solo el puntero PWM1. */
#ifndef TPI_TEST_PWM_REGISTROS_H
#define TPI_TEST_PWM_REGISTROS_H

#include "LPC17xx.h"

extern LPC_PWM_TypeDef tpi_pwm_modelo;

#undef LPC_PWM1
#define LPC_PWM1 (&tpi_pwm_modelo)

#endif /* TPI_TEST_PWM_REGISTROS_H */

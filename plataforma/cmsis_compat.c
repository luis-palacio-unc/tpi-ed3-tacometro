/**
 * @file cmsis_compat.c
 * @brief Verificacion de compatibilidad de los encabezados CMSIS existentes.
 * @details Se compila para ARM sin configurar ni acceder a perifericos.
 * @note Soporte de plataforma B00, sin ESW adicional del anteproyecto.
 */
#include "LPC17xx.h"

#if !defined(__ARM_ARCH_7M__)
#error "El firmware LPC1769 requiere Cortex-M3 (ARMv7-M)."
#endif

_Static_assert(sizeof(uint32_t) == 4U, "Se requieren palabras de 32 bits.");
_Static_assert(DMA_IRQn == 26, "Revisar la tabla de vectores DMA.");
_Static_assert(CANActivity_IRQn == 34, "Se requieren 35 IRQ externas LPC1769.");

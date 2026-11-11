/**
 * @file Reloj.c
 * @brief Tiempo de acceso a flash, SystemInit y actualizacion de SystemCoreClock.
 * @details Unico punto del firmware que configura el reloj del sistema.
 * @note Soporte de plataforma B18/I1. Unico acceso directo a registros: FLASHCFG.
 */
#include "Reloj.h"
#include "LPC17xx.h"

/** @brief Campo FLASHTIM de FLASHCFG, bits 15:12 (UM10360 tabla 49). */
#define RELOJ_FLASHTIM_MASCARA UINT32_C(0x0000F000)
/** @brief FLASHTIM = 4: cinco ciclos de CPU por acceso, hasta 100 MHz. */
#define RELOJ_FLASHTIM_5_CICLOS UINT32_C(0x00004000)

void Reloj_Init(void)
{
    LPC_SC->FLASHCFG = (LPC_SC->FLASHCFG & ~RELOJ_FLASHTIM_MASCARA) | RELOJ_FLASHTIM_5_CICLOS;
    SystemInit();
    SystemCoreClockUpdate();
}

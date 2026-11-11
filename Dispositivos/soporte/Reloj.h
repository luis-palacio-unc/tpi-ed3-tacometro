/**
 * @file Reloj.h
 * @brief Arranque unico del reloj del sistema: flash, PLL0 y SystemCoreClock.
 * @details Envuelve SystemInit de la biblioteca original para fijar antes el tiempo
 * de acceso a flash que exige CCLK = 100 MHz. Sirve igual con el startup propio
 * (CMake) y con el de MCUXpresso.
 * @note Soporte de plataforma B18/I1, sin ESW propio en el anexo B.
 */
#ifndef TPI_RELOJ_H
#define TPI_RELOJ_H

/**
 * @brief Llevar CCLK a 100 MHz con acceso a flash valido en todo momento.
 * @details En este orden:
 * -# FLASHCFG = (FLASHCFG & ~0x0000F000) | 0x00004000: FLASHTIM = 4, cinco ciclos
 *    de CPU por acceso (UM10360 cap. 5, tabla 49). El valor de reset (FLASHTIM = 3,
 *    cuatro ciclos) solo vale hasta 80 MHz y SystemInit lo corrige recien despues
 *    de conectar el PLL0; cinco ciclos son validos a 100 MHz y tambien con el IRC
 *    de 4 MHz del arranque. Acceso directo a LPC_SC porque la biblioteca no ofrece
 *    esta escritura por separado; se conservan los bits 11:0.
 * -# SystemInit de system_LPC17xx.c: oscilador principal de 12 MHz, PLL0 con
 *    PLL0CFG = 0x00050063 (FCCO 400 MHz), CCLKCFG = 3 (CCLK 100 MHz), PCLKSEL0/1 = 0
 *    (PCLK = CCLK/4 = 25 MHz) y FLASHCFG otra vez en 5 ciclos (cap. 4, tablas 38 y
 *    40-42).
 * -# SystemCoreClockUpdate: recalcula SystemCoreClock desde los registros, que es
 *    el valor que usa la biblioteca para prescalers, baudios y reloj del ADC.
 * @pre Llamar una sola vez, desde reset y con el PLL0 desconectado, como primera
 * accion de main y antes de cualquier *_Init: reconfigurar el PLL0 conectado viola
 * el manual. En MCUXpresso no definir __USE_CMSIS, para que cr_startup no llame
 * tambien a SystemInit. El startup propio no configura el reloj.
 * @note Soporte de plataforma B18/I1. Bloquea dentro de SystemInit hasta que el
 * oscilador y los PLL enganchan; sin interrupciones. No acredita el reloj fisico.
 */
void Reloj_Init(void);

#endif /* TPI_RELOJ_H */

/**
 * @file Adquisicion_ADC_DMA.h
 * @brief Adquisicion continua de potenciometro y LM35 mediante ADC y DMA.
 * @details El soporte configura el ADC en rafaga sobre AD0.0/AD0.1 y el canal 0
 * del GPDMA, que copia cada resultado a la muestra de su dispositivo. Cada
 * dispositivo posee su muestra; este encabezado declara los destinos.
 * @note Soporte compartido de ESW.2.1.11 (LM35) y ESW.2.1.12 (Potenciometro), sin
 * ESW propio en el anexo B. Uso interno de Dispositivos y de la inicializacion.
 */
#ifndef TPI_ADQUISICION_ADC_DMA_H
#define TPI_ADQUISICION_ADC_DMA_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Palabra AD0DR0 transferida por DMA para AD0.0/P0.23.
 * @details Definida una sola vez en Potenciometro.c. El soporte usa su direccion
 * como destino DMA. Son 32 bits, alineados naturalmente a 4 bytes.
 * RESULT ocupa bits 15:4; Pot_Read extrae las 12 cuentas con el driver original.
 * @note Uso interno de dispositivos. El DMA es el unico escritor en ejecucion;
 * la CPU consulta volatile. Se ubica en .bss de SRAM local, accesible por GPDMA
 * con el microcontrolador activo. No representa una lectura disponible al arrancar:
 * consultar Adquisicion_ADC_DMA_HayDatos.
 */
extern volatile uint32_t ADC_DMA_Pot;

/**
 * @brief Palabra AD0DR1 transferida por DMA para AD0.1/P0.24 (LM35).
 * @details Definida una sola vez en LM35.c; el dispositivo consulta RESULT (15:4).
 * @note Uso interno. Cero inicial no indica disponibilidad: consultar
 * Adquisicion_ADC_DMA_HayDatos.
 */
extern volatile uint32_t ADC_DMA_LM35;

/**
 * @brief Configurar y arrancar ADC en rafaga y DMA circular para ambas muestras.
 * @details Con la biblioteca original y en este orden (UM10360 cap. 29 y 31):
 * -# NVIC_DisableIRQ(ADC_IRQn): con DMA la interrupcion del ADC debe quedar
 *    deshabilitada en el NVIC (seccion 29.6.4).
 * -# ADC_Init(1500): PCONP.PCADC (tabla 46) y AD0CR = PDN | CLKDIV = 255 (tabla
 *    532). Con PCLK_ADC = 25 MHz el reloj del ADC es 25 MHz/256 = 97,7 kHz (limite
 *    13 MHz); en rafaga cada conversion usa 64 ciclos: 1526 conversiones/s, 763
 *    pares/s.
 * -# ADC_PinConfig canales 0 y 1: P0.23 = AD0.0 y P0.24 = AD0.1, funcion 01 de
 *    PINSEL1 (tabla 81) y PINMODE1 = 10, sin pull-up ni pull-down (seccion 29.6.3).
 * -# ADC_ChannelEnable 0 y 1: AD0CR.SEL = 0x03.
 * -# ADC_IntDisable(GLOBAL) y ADC_IntEnable canales 0 y 1: AD0INTEN = 0x003; en
 *    rafaga ADGINTEN debe ser 0 (tabla 534). El pedido DMA es la linea de
 *    interrupcion del ADC: se activa con el DONE de cada canal habilitado y se
 *    retira al leer su AD0DRn (secciones 29.6.2 y 29.6.4).
 * -# GPDMA_Init: PCONP.PCGPDMA y habilitacion del controlador (tabla 558).
 * -# Lista enlazada circular de dos descriptores de 1 palabra de 32 bits, rafaga
 *    DMA de 1, sin incremento (seccion 31.6.5, tabla 564): AD0DR0 -> ADC_DMA_Pot y
 *    AD0DR1 -> ADC_DMA_LM35; solo el segundo pide interrupcion de fin de cuenta.
 * -# GPDMA_SetupChannel canal 0, periferico a memoria con pedido del ADC (pedido
 *    4, tabla 544; DMACCConfig tabla 565). La biblioteca fuerza AD0GDR como origen
 *    y un unico criterio de interrupcion; por eso se cargan despues, por acceso
 *    directo, DMACC0SrcAddr y DMACC0Control con el primer descriptor (paso 3 de
 *    la seccion 31.6.5.1.1). Leer siempre AD0DRn y nunca AD0GDR respeta la regla
 *    de usar un solo metodo de lectura (secciones 29.5.2 y 29.5.4) y fija la
 *    asociacion canal-destino aunque se pierda un pedido.
 * -# NVIC: DMA_IRQn con prioridad 3 y habilitada; GPDMA_ChannelStart;
 *    ADC_BurstEnable al final (AD0CR.BURST = 1 con START = 000, tabla 532).
 * @return true si la adquisicion quedo en marcha; false si GPDMA_SetupChannel
 * rechazo el canal por estar ya habilitado, en cuyo caso no se arranca nada.
 * @pre SystemInit ejecutado y SystemCoreClock = CCLK real. ADC y GPDMA exclusivos;
 * llamar una vez desde reset, desde el lazo privilegiado. VREFP = ADC_VREF_MV.
 * @note Soporte de ESW.2.1.11/ESW.2.1.12. GPDMA_Init espera el bit de habilitacion
 * del controlador solo al iniciar. La interrupcion de error del canal queda
 * enmascarada: un error de bus detendria el canal y el contador de pares.
 */
bool Adquisicion_ADC_DMA_Init(void);

/**
 * @brief Informar si ya se completo al menos un par de muestras desde Init.
 * @return true desde la primera interrupcion de fin de cuenta del segundo
 * descriptor: ADC_DMA_Pot y ADC_DMA_LM35 contienen conversiones reales. No se
 * vuelve a false hasta una nueva inicializacion.
 * @note Soporte de ESW.2.1.11/ESW.2.1.12. Lectura de una bandera volatile escrita
 * por el ISR de DMA; sin seccion critica ni esperas. No acredita frescura.
 */
bool Adquisicion_ADC_DMA_HayDatos(void);

/**
 * @brief Contar los pares de muestras completados desde Init.
 * @return Contador uint32_t modulo 2^32 de pares (potenciometro, LM35) transferidos;
 * avanza unas 763 veces por segundo. Si deja de avanzar, la adquisicion se detuvo.
 * @note Soporte de ESW.2.1.11/ESW.2.1.12. Lectura volatile de 32 bits escrita por
 * el ISR de DMA; sin seccion critica. Restar lecturas para medir frescura.
 */
uint32_t Adquisicion_ADC_DMA_ContarPares(void);

#endif /* TPI_ADQUISICION_ADC_DMA_H */

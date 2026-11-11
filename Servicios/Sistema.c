/**
 * @file Sistema.c
 * @brief Arranque del reloj y de la adquisicion ADC/DMA con espera acotada.
 * @details Unico servicio que usa el soporte Reloj y Adquisicion_ADC_DMA.
 * @note Soporte de plataforma B18/I2, sin ESW propio en el anexo B.
 */
#include "Sistema.h"
#include "Adquisicion_ADC_DMA.h"
#include "Reloj.h"
#include <stdint.h>

/**
 * @brief Consultas maximas de disponibilidad antes de declarar fallo.
 * @details A 100 MHz cada consulta lleva decenas de ciclos: el limite equivale a
 * no menos de 100 ms, frente a los 2 ms que tarda el primer par de muestras.
 */
#define SISTEMA_ESPERA_ADC_CONSULTAS UINT32_C(2000000)

bool Sistema_Init(void)
{
    Reloj_Init();
    if (!Adquisicion_ADC_DMA_Init()) {
        return false;
    }
    for (uint32_t i = 0U; i < SISTEMA_ESPERA_ADC_CONSULTAS; ++i) {
        if (Adquisicion_ADC_DMA_HayDatos()) {
            return true;
        }
    }
    return false;
}

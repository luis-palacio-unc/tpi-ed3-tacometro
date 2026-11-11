/**
 * @file Temperatura.c
 * @brief Promedio movil de las lecturas del LM35.
 * @details Ventana circular de cuatro lecturas en memoria estatica.
 * @note ESW.2.1.4. Contrato publico en Temperatura.h.
 */
#include "Temperatura.h"
#include "LM35.h"

/** @brief Ultimas lecturas del LM35, en centesimas de grado. */
static int32_t Temperatura_Lecturas[TEMPERATURA_MUESTRAS];
/** @brief Lecturas validas en la ventana: 0 al arrancar, hasta TEMPERATURA_MUESTRAS. */
static uint32_t Temperatura_Cantidad;
/** @brief Posicion de la proxima lectura dentro de la ventana. */
static uint32_t Temperatura_Siguiente;

int32_t Temperatura_Leer(void)
{
    Temperatura_Lecturas[Temperatura_Siguiente] = LM35_Read();
    Temperatura_Siguiente = (Temperatura_Siguiente + 1U) % TEMPERATURA_MUESTRAS;
    if (Temperatura_Cantidad < TEMPERATURA_MUESTRAS) {
        ++Temperatura_Cantidad;
    }
    int32_t suma = 0;
    for (uint32_t i = 0U; i < Temperatura_Cantidad; ++i) {
        suma += Temperatura_Lecturas[i];
    }
    return suma / (int32_t)Temperatura_Cantidad;
}

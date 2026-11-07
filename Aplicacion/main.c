/**
 * @file main.c
 * @brief Entrada minima de la aplicacion del tacometro.
 * @details B00 mantiene un lazo sin inicializacion de servicios o perifericos.
 * @note ESW.2.1.1. El startup inicializa .data y .bss antes de entrar aqui.
 */
#include "Procesamiento.h"

/**
 * @brief Mantener la aplicacion en el lazo minimo de B00.
 * @details No retorna ni modifica perifericos; pendiente de integrar servicios.
 * @return No retorna; mantiene el lazo minimo hasta integrar los servicios.
 * @note USW.2.1.1.1.
 */
int main(void)
{
    for (;;) {
    }
}

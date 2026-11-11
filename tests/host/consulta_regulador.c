/* Emisor para el oraculo del regulador: una linea "consigna rpm_milesimas" por paso. */
#include "Procesamiento.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    uint32_t consigna, valor;
    int campos;
    while ((campos = scanf("%" SCNu32 " %" SCNu32, &consigna, &valor)) == 2) {
        printf("%" PRIu16 "\n", Procesamiento_Regular(consigna, valor));
        fflush(stdout);
    }
    return campos == EOF ? 0 : 2;
}

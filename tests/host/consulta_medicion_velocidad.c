#include "Medicion_Velocidad.h"
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>

int main(void)
{
    uint32_t pulsos, duracion_us, factor;
    int leidos;
    while ((leidos = scanf("%" SCNu32 " %" SCNu32 " %" SCNu32,
                            &pulsos, &duracion_us, &factor)) == 3) {
        Medicion_Velocidad_Lectura lectura;
        Medicion_Velocidad_Estado estado =
            Medicion_Velocidad_Actualizar(&lectura, pulsos, duracion_us, factor);
        const char *nombre;
        switch (estado) {
            case MEDICION_VELOCIDAD_VALIDA: nombre = "VALIDA"; break;
            case MEDICION_VELOCIDAD_ERROR_DURACION: nombre = "ERROR_DURACION"; break;
            case MEDICION_VELOCIDAD_ERROR_PULSOS_POR_VUELTA: nombre = "ERROR_FACTOR"; break;
            case MEDICION_VELOCIDAD_ERROR_RANGO: nombre = "ERROR_RANGO"; break;
            default: return 2;
        }
        if (lectura.estado != estado) {
            return 3;
        }
        printf("%s %" PRIu32 "\n", nombre, lectura.rpm_milesimas);
    }
    return leidos == EOF ? 0 : 1;
}

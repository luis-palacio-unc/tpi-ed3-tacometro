/**
 * @file Medicion_Velocidad.c
 * @brief Conversion de los pulsos de cada ventana a milesimas de RPM.
 * @details Sin estado propio: la ventana la cierra el ISR del Timer.
 * @note ESW.2.1.3. Contratos publicos en Medicion_Velocidad.h.
 */
#include "Medicion_Velocidad.h"
#include "Control.h"
#include "Medicion.h"
#include "Sensor_Optico.h"
#include "Timer.h"

/** @brief Milisegundos por minuto, para pasar de pulsos por ventana a RPM. */
#define MEDICION_MS_POR_MINUTO UINT64_C(60000)

void Medicion_Velocidad_Init(void)
{
    SensorOptico_Init();
    Timer_Init();
    Timer_Start();
}

bool Medicion_Velocidad_Actualizar(uint32_t *rpm_milesimas)
{
    uint32_t pulsos;
    if (!Timer_ConsumirVentana(&pulsos)) {
        return false;
    }
    const uint64_t rpm = (uint64_t)pulsos * MEDICION_MS_POR_MINUTO * MEDICION_VELOCIDAD_ESCALA_RPM /
                         ((uint64_t)TPI_MEDICION_PULSOS_POR_VUELTA * TPI_CONTROL_PERIODO_MS);
    *rpm_milesimas = rpm > UINT32_MAX ? UINT32_MAX : (uint32_t)rpm;
    return true;
}

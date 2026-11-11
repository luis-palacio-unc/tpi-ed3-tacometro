# Delta B18 — temperatura

Candidato de la vía rápida B18. I1 cambió sólo el valor por defecto de ADC_VREF_MV
(decisión O2). I2 reemplaza el servicio genérico (TE2, y la parte de servicio de
TE3/TE4) por una lectura filtrada. TE1 y la conversión del dispositivo LM35 no cambian.

## MODIFIED Requirements

### Requirement: TE1 — Conversión específica en dispositivo
Sin cambios salvo: ADC_VREF_MV MUST conservar #ifndef/#error y su default MUST ser
3300 mV, correspondiente a VREFP alimentado a 3,3 V en la placa; el valor físico
MUST medirse en la puesta en marcha. (Antes: default provisional 2540 mV.)

#### Scenario: S2 — Overrides y configuración inválida
- Given target host sin override y probes con valores no positivos.
- When se consulta la muestra 1024.
- Then el default MUST producir 8250 centésimas; los probes inválidos MUST fallar
  por #error ADC_VREF_MV. (Antes: 6350.)

## REMOVED Requirements

### Requirement: TE2 — Servicio genérico en unidades físicas
Motivo (I2): `Temperatura_Inicializar/Actualizar` con estados VALIDA/SIN_DATOS/
ERROR_* sólo copiaban el valor del dispositivo. `Sistema_Init` garantiza una
conversión real antes de la primera lectura. Los escenarios S4–S7 de servicio
quedan como historial; S1–S3 y S6 del dispositivo LM35 siguen vigentes.

## ADDED Requirements

### Requirement: TE5 — Lectura filtrada
`Servicios/Temperatura` MUST exponer `int32_t Temperatura_Leer(void)`: toma
`LM35_Read()` y devuelve el promedio entero (truncado) de las últimas 4 lecturas,
en centésimas de °C; mientras haya menos de 4 MUST promediar las disponibles.

#### Scenario: S9 — Arranque del filtro (`i2.temperatura` S1)
- Given lecturas 2000, 3000, 4000 y 5000 desde el arranque.
- When se lee cuatro veces.
- Then MUST dar 2000, 2500, 3000 y 3500.

#### Scenario: S10 — Ventana de cuatro (`i2.temperatura` S2)
- Given la ventana anterior y luego lecturas constantes de 6000 y de 2501.
- When se lee.
- Then MUST dar 4500, 5250, 5750, 6000, 6000 y luego 5125, 4250, 3375, 2501.

#### Scenario: S11 — Servicio sobre el LM35 real (`i2.temperatura_lm35`, `b05.lm35_enlace_arm`)
- Given `LM35.c` real con ADC_VREF_MV = 2000 y palabras de ADC de 25,00 y 100,00 °C.
- When se lee por el servicio.
- Then MUST dar 2500, 6250, 7500, 8125 y 10000, sin modificar la muestra.

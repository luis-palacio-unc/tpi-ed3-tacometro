# Delta B18/I2 — medicion-velocidad (reemplaza el contrato vigente)

Candidato de la vía rápida B18. Los requisitos de B03/B03-R1 quedan como historial:
el servicio deja de recibir conteo, duración y factor, y toma la ventana del Timer.

## REMOVED Requirements

### Requirement: contrato B03/B03-R1 completo
Motivo: conversión genérica con seis estados y redondeo exacto para entradas
arbitrarias. En este firmware la duración (500 ms) y el factor (60) son constantes
de `Configuracion/`. Se eliminan también `tests/host/verificar_velocidad.py` y
`consulta_medicion_velocidad.c`.

## ADDED Requirements

### Requirement: MV1 — Arranque de la medición
`Servicios/Medicion_Velocidad` MUST exponer `void Medicion_Velocidad_Init(void)`
que llama `SensorOptico_Init`, `Timer_Init` y `Timer_Start`, en ese orden.

#### Scenario: S1 — Inicio en orden (`i2.medicion_velocidad`)
- Given dispositivos simulados.
- When se inicializa.
- Then MUST observarse exactamente esa secuencia.

### Requirement: MV2 — Una medición por ventana
MUST exponer `bool Medicion_Velocidad_Actualizar(uint32_t *rpm_milesimas)`. Si
`Timer_ConsumirVentana` entrega una ventana MUST escribir pulsos × 60 000 × 1000 /
(`TPI_MEDICION_PULSOS_POR_VUELTA` × `TPI_CONTROL_PERIODO_MS`) calculado en 64 bits,
saturado en UINT32_MAX, y devolver true; si no, MUST devolver false sin escribir.
MUST NOT recortar a 185 RPM. El retorno true es el periodo del lazo de control.

#### Scenario: S2 — Pulsos a milésimas de RPM (`i2.medicion_velocidad`)
- Given ventanas de 0, 1, 50, 92, 93, 2 147 483, 2 147 484 y UINT32_MAX pulsos.
- When se actualiza.
- Then MUST dar 0, 2000, 100 000, 184 000, 186 000, 4 294 966 000 y UINT32_MAX en
  los dos últimos.

#### Scenario: S3 — Una vez por ventana (`i2.medicion_velocidad`)
- Given sin ventana, luego una ventana, luego ninguna.
- When se actualiza tres veces.
- Then MUST devolver false, true y false, escribiendo el destino sólo en el true.

### Requirement: MV3 — Tipo de entrada del regulador B17
`Medicion_Velocidad.h` MUST conservar `MEDICION_VELOCIDAD_ESCALA_RPM`,
`Medicion_Velocidad_Estado` y `Medicion_Velocidad_Lectura` sin cambios de valores
ni de campos, porque `Procesamiento_Regular` (B17, intacto) los recibe.

#### Scenario: S4 — B17 intacto (`b17.regulador`, `b17.oraculo_planta`)
- Given las pruebas y el oráculo de B17 sin modificar.
- When se compilan contra el encabezado nuevo.
- Then MUST pasar sin cambios.

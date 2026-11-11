# Delta B18/I2 — mandos (reemplaza el contrato vigente)

Candidato de la vía rápida B18. MA1–MA4 (B04) y SE1–SE4 (B10) quedan como historial:
el servicio pasa a leer sus dispositivos y deja de recibir muestras, fondos de
escala, tiempos y contextos.

## REMOVED Requirements

### Requirement: MA1–MA4 y SE1–SE4
Motivo: normalización genérica con estados de error, y antirrebote temporal con
contexto, sin caso real en este firmware. El rebote lo filtra leer la llave una vez
por periodo de 500 ms.

## ADDED Requirements

### Requirement: MA5 — Perilla como mando
`Servicios/Mandos` MUST exponer `uint16_t Mandos_LeerManual(void)` que devuelve
(`Pot_Read()` × 1000 + 2047) / 4095: 0..1000 por mil, redondeado al cercano,
monótono, sin estado.

#### Scenario: S1 — Perilla a por mil (`i2.mandos`)
- Given `Pot_Read` simulado en 0, 1, 2, 3, 2047, 2048, 2050, 4092, 4093 y 4095.
- When se lee el mando.
- Then MUST dar 0, 0, 0, 1, 500, 500, 501, 999, 1000 y 1000; en todo el recorrido
  0..4095 MUST ser no decreciente, con pasos de a lo sumo 1.

### Requirement: MA6 — Llave como modo
MUST exponer `void Mandos_Init(void)` (llama `Selector_Init`) y
`Mandos_Modo Mandos_LeerModo(void)`: `Selector_Read()` verdadero (llave abierta,
pull-up) → `MANDOS_MODO_MANUAL`; falso → `MANDOS_MODO_AUTOMATICO`. Sin antirrebote propio.

#### Scenario: S2 — Llave a modo (`i2.mandos`)
- Given selector simulado en alto y en bajo.
- When se inicializa y se lee el modo.
- Then MUST inicializar sólo el selector y dar MANUAL y AUTOMATICO respectivamente.

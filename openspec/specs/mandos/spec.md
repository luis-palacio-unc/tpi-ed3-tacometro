# Mandos — contrato vigente

Origen: R03/R05/R06/R10/R11/R15 y contrato de modos B01. Contratos anteriores en el historial archivado.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

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

# Mando de motor — contrato vigente

Origen R03/R06/R10/R11/R15; resultado B08 aceptado por usuario el 2026-11-09. Contratos anteriores en el historial archivado.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: MM5 — Mando limitado
`Servicios/Driver_Motor` MUST exponer `void Driver_Motor_Init(void)` (llama
`Motor_Init`, que arranca con mando 0) y `void Driver_Motor_Aplicar(uint16_t
por_mil)`, que entrega a `Motor_Write` el valor limitado a 1000. Sin estado.

#### Scenario: S1 — Mando limitado a 1000 (`i2.drivers`)
- Given motor simulado.
- When se inicializa y se aplican 0, 1, 500, 999, 1000, 1001, 65535 y 0.
- Then Init MUST NOT escribir mando; cada aplicación MUST producir una escritura
  con 0, 1, 500, 999, 1000, 1000, 1000 y 0, y `Motor_Write` MUST NOT recibir más de 1000.

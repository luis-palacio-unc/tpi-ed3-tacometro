# Alarma de temperatura — contrato vigente

Origen R07/R10/R11/R15; B06 aceptado por el usuario el 2026-11-09. Contratos anteriores en el historial archivado.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: AT5 — Alarma con histeresis
`Configuracion/Alarma.h` MUST definir, con `#ifndef` y control por `#error`,
`TPI_ALARMA_UMBRAL_CENTESIMAS` = 4500 y `TPI_ALARMA_HISTERESIS_CENTESIMAS` = 200.
`Aplicacion/Procesamiento` MUST exponer `bool Procesamiento_EvaluarAlarma(int32_t
temperatura_centesimas)`: MUST encender con T > umbral, apagar con T ≤ umbral −
histéresis y conservar la decisión anterior entre ambos. MUST arrancar apagada.
MUST NOT depender del modo ni detener el motor.

#### Scenario: S1 — Histéresis 45/43 (`i2.procesamiento`)
- Given la secuencia 44,00; 45,00; 45,01; 45,00; 44,00; 43,01; 43,00; 43,01; 45,00;
  90,00; 43,00; 45,01; −5,00; 45,01; 0,00 °C desde el arranque.
- When se evalúa cada valor.
- Then MUST dar apagada, apagada, encendida ×4, apagada ×3, encendida, apagada,
  encendida, apagada, encendida, apagada.

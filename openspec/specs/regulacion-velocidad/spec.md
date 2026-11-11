# Regulación de velocidad — contrato vigente

Origen R04/R06/R10/R11/R14/R15, ESW.2.1.2. Contratos anteriores en el historial archivado.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: RG1 — Paso del PI
`Aplicacion/Procesamiento` MUST exponer
`uint16_t Procesamiento_Regular(uint32_t consigna_rpm, uint32_t rpm_milesimas)`
(USW.2.1.2.9). Un paso representa 500 ms. Error = consigna × 1000 − medición, en
milésimas de RPM, sin recortar la medición. Mando = Kp × error + integral,
limitado a 0..1000 y redondeado una sola vez al más cercano (el empate sube).
Las ganancias MUST ser las fracciones `TPI_CONTROL_KP_NUM/KP_DEN` y
`TPI_CONTROL_KI_NUM/KI_DEN` de `Control.h`. Sin coma flotante; aritmética de 64
bits sin desborde para cualquier entrada de 32 bits.

#### Scenario: S1 — Paso y empate (`b17.regulador`)
- Given integral en cero y ganancias por defecto (Kp 4/1000, Ki 1/1000).
- When regula 100 RPM dos veces con medición 0; y, desde cero, 1 RPM con 875 y 876.
- Then MUST dar 400 y 500; 1 (empate 0,5) y 0.

### Requirement: RG2 — Integral y anti-windup
Después de calcular el mando con la integral previa MUST sumar Ki × error a la
integral, limitada a 0..1000, excepto si el mando sin limitar es ≥ 1000 con error
positivo o ≤ 0 con error negativo: entonces MUST conservarla.

#### Scenario: S2 — Desaturación (`b17.regulador`)
- Given 185 RPM con medición 0: dos pasos (740, 925) y 40 pasos saturados en 1000.
- When la consigna baja a 100 con igual medición.
- Then MUST dar 770; con medición `UINT32_MAX` 0, y luego 870.

#### Scenario: S3 — Mando exactamente cero (`b17.regulador`)
- Given cuatro pasos de 1 RPM con medición 0 (integral 4).
- When mide 2000 y luego 1000 milésimas.
- Then MUST dar 0 sin integrar y luego 4.

### Requirement: RG3 — Reinicio
La integral vive en memoria estática y MUST quedar en cero con consigna 0 (mando
0), con `Procesamiento_InicializarModo` y con `Procesamiento_ActualizarModo` en
modo MANUAL. Una consigna nueva en automático MUST NOT borrarla.

#### Scenario: S4 — Reinicios (`b17.regulador`, `i2.aplicacion` S4)
- Given integral distinta de cero.
- When llega consigna 0, o modo MANUAL, o se inicializa el modo.
- Then el siguiente paso de 100 RPM con medición 0 MUST dar 400; tras una consigna
  nueva en automático MUST dar 500.

### Requirement: RG4 — Configuración
`Control.h` MUST proteger cada componente de ganancia en 1..1000 con
`#ifndef`/valor por defecto/`#error`, y `TPI_CONTROL_PERIODO_MS` MUST ser 500. Los
valores por defecto son provisionales hasta sintonizar con el motor.

#### Scenario: S5 — Probes (`b17.configuracion_control`)
- Given 4 configuraciones válidas y 18 inválidas (0, −1, 1001, `UINT32_MAX` por
  componente; periodo 499 y 1000).
- When se compilan.
- Then las válidas MUST compilar con sus valores y las inválidas fallar con el
  `#error` de su macro.

### Requirement: RG5 — Evidencia
Un oráculo racional independiente MUST coincidir paso a paso con el C para 8
juegos de ganancias compilados aparte, incluidos dos con Ki > Kp que alcanzan los
límites de la integral. Con las ganancias por defecto, la planta de prueba MUST
cumplir sobrepico ≤ 10 % del escalón y establecimiento ≤ 20 pasos a ±2 RPM en
0→100→185. El fixture ARM `b17.enlace_arm` MUST enlazar sólo
`Procesamiento_Regular`, sin auxiliares de coma flotante.

#### Scenario: S6 — Oráculo y planta (`b17.oraculo_planta`)
- Given secuencias deterministas y aleatorias por juego de ganancias.
- When se compara cada mando con el de la referencia en fracciones.
- Then MUST coincidir en todos los pasos y cumplir las métricas de la planta.

# Delta B18/I2 — regulacion-velocidad (sólo composición)

Candidato de la vía rápida B18. RV1–RV4 y RV6, la API, los tipos, `Control.h`, las
pruebas `test_regulador.c`, el oráculo y `verificar_configuracion_control.py` no
cambian. Sólo cambia quién compone el regulador.

## MODIFIED Requirements

### Requirement: RV5 — Composición y desempeño simulado
La composición MUST probarse en la suite de sistema `i2.aplicacion` (contrato
sistema SI4): AUTO_ESPERA y MANUAL reinician el PI y sólo AUTO_ACTIVO regula, con
la medición entregada como VALIDA. El fixture ARM `b17.enlace_arm` MUST retener
sólo inicializar/regular/reiniciar. Las métricas de desempeño del modelo entero
(S8) no cambian. (Antes: fixture propio B01 + PI + Driver_Motor con proveedor
emulado, `test_integracion_regulador.c`.)

#### Scenario: S7 — Transiciones
- Given manual → automático → consigna → manual → reentrada.
- When se ejecuta `i2.aplicacion` S2/S4.
- Then MUST aplicar un mando por periodo, 0 en espera, y reiniciar la historia del
  PI: tras la reentrada el primer mando con consigna 100 y eje detenido MUST ser 400.

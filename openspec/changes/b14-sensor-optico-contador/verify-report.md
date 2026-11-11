# Verificación prevista B14

Propuesta sin acuerdo/implementación. No se ejecutaron pruebas de B14.
Exploración sólo de lectura confirma APIs y pin/contador en manual. No es evidencia
de que el sensor físico se esté contando ni de que SensorOptico_Read ya exista.

| Escenario | Evidencia a producir al implementar |
|---|---|
| S1/S2 |Conteos íntegros0/MAX, TIMER0 independiente y repetición/avance sin reset |
| S3 |Fixture B03 con diferencias0/60/185/200 por1s/factor60, cero válido y sin recorte |
| S4 |MAX-29→30, diferencia60 y medición60000 milésimasRPM |
| S5 |Host/san, ARM/header/símbolos/recursos, Doxygen/diff/hashes; sin hardware |

Comandos previstos: CMake host externo y build/ctest; build sanitizado con
-DTPI_HOST_SANITIZERS=ON y detección de fugas; ARM -DTPI_BUILD_ARM=ON con toolchain
cmake/arm-none-eabi.cmake; Doxygen con salida externa. Registrar comandos/exits/logs
reales al ejecutar. No validar la biblioteca copiando su implementación a tests.

Inicialización TIMER1/pinmux/reloj/CCR y señal/contador reales, ventanas/main/servicios
siguen para integración final. B13 aceptado es antecedente, no prueba de este bloque.

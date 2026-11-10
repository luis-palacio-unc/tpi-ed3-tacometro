# Verificación prevista B07-R1

Propuesta sin acuerdo, código sin modificar. No se ejecutaron pruebas B07-R1.
Revisión estática: Driver_Alarmas exige bool(void*,bool); B12 acepta void(bool).
El contrato vigente salidas-alarma conserva B07 hasta aceptación de esta revisión.

| Escenarios | Evidencia a producir al implementar |
|---|---|
| S1/S2 | Secuencia/valor/repetición y dos pares de funciones sin cruce |
| S3/S4/S5 | Todas las interfaces incompletas sin llamadas, recuperación e inmutabilidad |
| S6/S7 | Integración B06 conserva decisiones/diagnóstico sin orden ante ausencia/error |
| S9 | Driver_Alarmas real + LED/Buzzer reales + doble GPIO, sin adaptadores/casts |
| S8 | Host/san, ARM, aislamiento, Doxygen, recursos, hashes y diff |

Comandos previstos con salida externa: cmake -S "$R" -B "$O/host" y build/ctest;
otro build con -DTPI_HOST_SANITIZERS=ON, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;
ARM -DTPI_BUILD_ARM=ON y -DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake";
build/ctest para imagen/fixture, sin ejecutarlos en MCU. Doxygen HTML/XML estricto
con OUTPUT_DIRECTORY externo. Guardar comandos/resultados reales al ejecutarlos.

Hardware, inicialización/enlace global, main y ensayos eléctricos diferidos.
La evidencia aceptada B12 es antecedente; no acredita que B07-R1 esté implementado.

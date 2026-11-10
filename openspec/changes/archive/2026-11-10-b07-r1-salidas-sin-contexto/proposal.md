# B07-R1 — Simplificar la interfaz de salidas de alarma

Estado: alcance/criterio acordados2026-11-10; implementación autorizada, resultado pendiente de aceptación.

## Objetivo y origen

R07/R10/R11/R15 y corrección del usuario2026-11-10: las salidas GPIO B12 no usan
contexto ni pueden informar fallos. B07 aún exige bool(void*,bool), incompatible
con LED_Write/Buzzer_Write void(bool). Revisar ese contrato sin ligar el servicio al MCU.

## Alcance y enfoque

- Cambiar Driver_Alarmas_EscribirSalida a void (*)(bool activar).
- Driver_Alarmas_Salidas conserva únicamente escribir_led y escribir_buzzer.
- Mantener Aplicar(salidas,activar), validación completa previa y orden LED→buzzer,
  una llamada por canal, incluso al repetir órdenes; sin contexto ni estado propio.
- Retirar ERROR_LED/ERROR_BUZZER/ERROR_AMBOS. Conservar ACEPTADA y ERROR_ARGUMENTO:
  informan despacho de la solicitud o interfaz incompleta, nunca diagnóstico físico.
- Migrar pruebas/fixture ARM y demostrar composición directa con B12 en host,
  sin adaptadores/casts. Mantener decisión térmica B06 y ausencia/error sin órdenes.
- Actualizar ESW.2.1.6/USW.2.1.6.1 con Doxygen y seguimiento SDD.

Archivos: Servicios/Driver_Alarmas.h/.c; tests/host/test_driver_alarmas.c,
test_integracion_alarmas.c y test_dispositivos_gpio.c; cmake/verificar_enlace_b07.cmake;
CMakeLists.txt, README y documentación SDD/Doxygen. Drivers y código B12 intactos.

Excluye inicialización/enlace global/main, hardware real, alarmas nuevas, umbral,
Motor/B08, GPIO de confirmación y cualquier modificación de bibliotecas.

## Dependencias, riesgos y aceptación

B07/B06/B12 aceptados; toolchain existente. La ruptura de API exige migrar todos
los consumidores en el mismo bloque. No conservar firmas antiguas por compatibilidad.

1. Activar/desactivar/repetir transmite el mismo bool a ambas salidas en orden.
2. Interfaz NULL o cualquiera de sus funciones ausente: ERROR_ARGUMENTO sin salidas;
   llamada válida posterior: ACEPTADA. No existen contextos ni errores de proveedor.
3. Funciones B12 conectan directamente; polaridades y pines vecinos correctos en host.
4. B06 conserva decisiones/errores; servicio compila aislado de hardware/aplicación.
5. Host/sanitizadores, build/fixture ARM, regresiones y Doxygen pasan, con evidencia
   separada del ensayo físico pendiente; diff y biblioteca intactos.

Reversión: commit inverso sólo de B07-R1, conservando B12/historia; reaparece la
incompatibilidad previa. Commit local al terminar el bloque acordado; sin push.

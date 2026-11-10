# Tareas candidatas B07-R1

## 1. Acuerdo y contrato

- [x] 1.1 Acordar propuesta/escenarios; releer ESTADO/Git y preservar hashes Drivers/B06/B12.
- [x] 1.2 Revisar llamadores de Driver_Alarmas y confirmar alcance de migración en pruebas/fixture.

## 2. Servicio y consumidores

- [x] 2.1 Cambiar Driver_Alarmas.h/.c a void(bool), dos funciones y dos estados; conservar validación previa/orden.
- [x] 2.2 Actualizar Doxygen ESW.2.1.6/USW.2.1.6.1, sin contexto ni errores de proveedor.
- [x] 2.3 Migrar test_driver_alarmas.c: S1–S5 con orden/valores/argumentos/recuperación/interfaz inmutable.
- [x] 2.4 Migrar test_integracion_alarmas.c: conservar S6/S7 y separar errores de argumento de fallos térmicos.
- [x] 2.5 Agregar S9 a test_dispositivos_gpio.c y servicio en su target CMake, sin adaptadores ni casts.
- [x] 2.6 Migrar callbacks/símbolos del fixture cmake/verificar_enlace_b07.cmake.

## 3. Verificación y cierre

- [x] 3.1 Ejecutar host/sanitizadores, build/fixture ARM, servicio/header aislado y regresiones (S8).
- [x] 3.2 Ejecutar Doxygen estricto y revisar recursos/diff/hashes Drivers/B06/B12.
- [x] 3.3 Registrar matriz S1–S9 real, actualizar README/contexto/estado y commit normal local, sin push.
- [x] 3.4 Dejar resultado pendiente de aceptación, sin sincronizar salidas-alarma ni archivar B07-R1.

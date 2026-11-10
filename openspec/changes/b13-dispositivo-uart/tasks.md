# Tareas propuestas B13 UART

## 1. Acuerdo y contrato

- [x] 1.1 Acordar alcance/recurso UART0 y criterio; releer ESTADO/Git, preservar biblioteca/servicios y archivos aceptados.
- [x] 1.2 Confirmar firmas y precondiciones usando UART_Send/Receive originales y manual NXP.

## 2. Dispositivo y pruebas

- [x] 2.1 Implementar UART_Read/UART_Write en UART.h/.c, validación NULL/cero y modo NONE_BLOCKING.
- [x] 2.2 Documentar ESW16/USW16.1/.2 y pines P0.2/P0.3; UART_BAUDRATE configurable, con validación/overrides de compilación (S7).
- [x] 2.3 Añadir test_uart.c: S1–S3 con RX binario, canarios, vacío/parcial/completo y recuperación.
- [x] 2.4 Añadir S4/S5: TX binario/cero/parcial/completo, origen inmutable y continuación externa sin duplicados.
- [x] 2.5 Integrar target/test host y Doxyfile, conservando controles y fuentes existentes.

## 3. Verificación y cierre

- [x] 3.1 Ejecutar host/sanitizadores, regresiones y compilación ARM del módulo con headers reales (S6).
- [x] 3.2 Verificar Doxygen, recursos/símbolos, diff y hashes de biblioteca/servicios sin modificar Drivers.
- [x] 3.3 Registrar evidencia por escenario, actualizar docs/agente/README y commit local al completar, sin push.
- [x] 3.4 Dejar resultado sin aceptar; no sincronizar/archivar ni avanzar otro bloque automáticamente.

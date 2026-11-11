# Tareas propuestas B14 Sensor_Optico

## 1. Acuerdo y contexto

- [ ] 1.1 Acordar alcance/TIMER1/CAP1.0/P1.18 y aceptación; releer ESTADO/Git y preservar biblioteca/módulos aceptados.
- [ ] 1.2 Confirmar lectura/conteo y precondiciones en driver/manual, sin inicializar hardware.

## 2. Dispositivo y verificación

- [ ] 2.1 Implementar SensorOptico_Read en Sensor_Optico.h/.c, con Doxygen ESW10/USW10.1 y sin estado/reset.
- [ ] 2.2 Añadir test_sensor_optico.c para S1/S2: valores exactos, TIMER0 independiente, lecturas/avance sin perturbación.
- [ ] 2.3 Añadir S3/S4: fixture de diferencias/B03 para0/60/185/200 eventos y rollover, sin lógica nueva de ventanas en producto.
- [ ] 2.4 Integrar target/test CMake y Doxyfile, conservando controles y fuentes existentes.

## 3. Cierre

- [ ] 3.1 Ejecutar host/sanitizadores/regresiones y compilar ARM/header; inspeccionar símbolos/recursos (S5).
- [ ] 3.2 Ejecutar Doxygen estricto y revisar diff/hashes Drivers/Timer/B03/otros dispositivos/archivos aceptados.
- [ ] 3.3 Registrar matriz real y límites físicos, actualizar docs/agente/README y commit local del bloque, sin push.
- [ ] 3.4 Dejar aceptación/sincronización/archivo pendientes; no avanzar integración ni otro bloque.

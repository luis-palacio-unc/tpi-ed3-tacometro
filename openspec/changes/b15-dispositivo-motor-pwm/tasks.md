# Tareas propuestas B15 — Motor

## 1. Acuerdo y base

- [ ] 1.1 Acordar alcance/PWM1.1/P2.0, firma Motor_Write y aceptación; releer ESTADO/Git y preservar cambios.
- [ ] 1.2 Confirmar manual/driver y precondiciones de período, 0/100 % y latch, sin configurar periféricos.

## 2. Dispositivo y pruebas

- [ ] 2.1 Implementar Motor.h/.c, validación/conversión y despacho original; Doxygen ESW9/USW9.1.
- [ ] 2.2 Crear tests/host/test_motor.c: S1/S2 con extremos/redondeo/resolución y límites uint32_t.
- [ ] 2.3 Cubrir S3/S4: errores sin llamadas, recuperación/repetición y conservación de otros recursos.
- [ ] 2.4 Integrar target/test estricto en CMakeLists.txt e INPUT en Doxyfile sin desactivar controles.

## 3. Verificación y cierre

- [ ] 3.1 Ejecutar host/sanitizadores/regresiones, ARM/header y recursos/símbolos, diferenciando modalidad (S5).
- [ ] 3.2 Ejecutar Doxygen estricto y revisar diff/hashes Drivers/módulos/archivos aceptados.
- [ ] 3.3 Registrar evidencia real en verify-report y docs/agente/README; commit del bloque según autorización.
- [ ] 3.4 Dejar aceptación/sincronización/archivo pendientes; no avanzar integración u otro bloque.

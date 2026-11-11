# Tareas propuestas B15 — Motor

## 1. Acuerdo y base

- [x] 1.1 Acordar alcance/PWM1.1/P2.0, firma Motor_Write y aceptación; releer ESTADO/Git y preservar cambios.
- [x] 1.2 Confirmar manual/driver y precondiciones de período, 0/100 % y latch, sin configurar periféricos.

## 2. Dispositivo y pruebas

- [x] 2.1 Implementar Motor.h/.c, validación/conversión y despacho original; Doxygen ESW9/USW9.1.
- [x] 2.2 Crear tests/host/test_motor.c: S1/S2 con extremos/redondeo/resolución y límites uint32_t.
- [x] 2.3 Cubrir S3/S4: errores sin llamadas, recuperación/repetición y conservación de otros recursos.
- [x] 2.4 Integrar target/test estricto en CMakeLists.txt e INPUT en Doxyfile sin desactivar controles.

## 3. Verificación y cierre

- [x] 3.1 Ejecutar host/sanitizadores/regresiones, ARM/header y recursos/símbolos, diferenciando modalidad (S5).
- [x] 3.2 Ejecutar Doxygen estricto y revisar diff/hashes Drivers/módulos/archivos aceptados.
- [x] 3.3 Registrar evidencia real en verify-report y docs/agente/README; commit del bloque según autorización.
- [x] 3.4 Dejar aceptación/sincronización/archivo pendientes; no avanzar integración u otro bloque.

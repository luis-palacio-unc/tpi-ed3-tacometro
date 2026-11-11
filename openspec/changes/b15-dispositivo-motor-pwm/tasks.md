# Tareas B15 — simplificación autorizada a MR0

Implementación anterior c952892, aún sin aceptar. Esta lista sigue la corrección
actual; su evidencia reemplaza el informe anterior sin editar historial archivado.

## 1. Contrato y alcance

- [x] 1.1 Registrar orden del usuario, Git/base y conservación de biblioteca/contratos aceptados.
- [x] 1.2 Actualizar propuesta/spec/diseño: una entrada, MR0 local estable y errores sin escritura.

## 2. Dispositivo y pruebas

- [x] 2.1 Corregir Motor.h/.c y Doxygen USW9.1: sólo mando y lectura directa de MR0.
- [x] 2.2 Crear modelo RAM con tipos CMSIS y preinclude privado de pruebas_motor, sin cambiar Drivers.
- [x] 2.3 Adaptar test_motor: período del modelo, cambio entre llamadas, errores/recuperación y conservación.
- [x] 2.4 Ejecutar B15 individual para S1/S2/S3/S4/S6, conservando tablas y recorridos previos.

## 3. Verificación y cierre

- [x] 3.1 Ejecutar host/san/regresiones y ARM/header reales O0/O2 sin redirección (S5).
- [x] 3.2 Ejecutar Doxygen, recursos/relocaciones, negativos externos y hashes de fuentes protegidas.
- [x] 3.3 Actualizar informe/README/docs-agente, revisar diff y crear commit local sin push.
- [x] 3.4 Dejar resultado sin aceptar/sincronizar/archivar; no avanzar integración ni otro bloque.

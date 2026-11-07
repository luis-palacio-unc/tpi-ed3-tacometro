# Tareas B00

## 1. Contrato y preparación
- [x] 1.1 Registrar acuerdo de alcance; inicializar Git en main, sin commits/push.
- [x] 1.2 Consultar mapa/boot NXP y redactar specs/build/spec.md y design.md.

## 2. Implementación
- [x] 2.1 Crear cmake/arm-none-eabi.cmake y perfiles en CMakeLists.txt/CMakePresets.json (S1/S2/S7).
- [x] 2.2 Crear plataforma/lpc1769.ld con límites, pila, checksum y CRP desactivado (S3/S5).
- [x] 2.3 Crear startup_lpc1769.S, cmsis_compat.c y main mínimo sin periféricos (S6).
- [x] 2.4 Implementar scripts/verificar_elf.py y reporte memoria.txt (S3/S4).

## 3. Verificación
- [x] 3.1 Crear scripts/pruebas_b00.py con casos negativos S2/S4/S5 y fixture S6.
- [x] 3.2 Ejecutar builds limpios host/ARM; CTest y revisión de símbolos/desensamblado (S1–S7).
- [x] 3.3 Revisar diff, preservar biblioteca/cambios previos y registrar evidencia en verify-report.md.

## 4. Cierre del alcance implementado
- [x] 4.1 Actualizar README, ESTADO, CONTEXTO, MAPA_REQUISITOS y config/state SDD.
- [x] 4.2 Informar resultado y limitaciones; detenerse sin B01, commits ni push.

Resultado aceptado por el usuario el 2026-11-06; especificaciones sincronizadas y cambio archivado en el cierre posterior.

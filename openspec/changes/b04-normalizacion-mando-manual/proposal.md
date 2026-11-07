# B04 — Normalización del mando manual

Estado: alcance acordado el 2026-11-06. Implementación y commit autorizados, sin push; resultado pendiente de aceptación.

## Objetivo y origen

Implementar la normalización de Mandos para convertir lecturas del potenciómetro al
mando lógico 0..1000 de Procesamiento. Origen: R03 (manual), B01 y R15 (portabilidad).

## Alcance y comportamiento

- Entradas: lectura entera y fondo de escala explícito de la adquisición; sin fijar
  cantidad de bits, referencia de tensión, canal ni microcontrolador en la aplicación.
- Salida: mando entero por mil, 0..1000 inclusive, con validez/error observable.
- Conversión lineal: cero → 0; fondo de escala → 1000. Redondeo al más cercano,
  empates hacia arriba; error aritmético máximo de 0,5 unidades por mil.
- Escala nula o lectura superior al fondo de escala: error explícito y salida
  invalidada, sin saturar ni presentar un mando anterior como nuevo válido.
- Mantener rango y contrato de B01; adaptación en aplicación, sin dependencia
  Servicios→Aplicacion. Documentar ESW/USW con Doxygen y verificar límites C.

Excluye ADC/DMA, configuración eléctrica, selector/antirrebote, filtrado/calibración,
PWM, driver de motor, regulación e integración del main. No acredita recorrido físico.

## Dependencias y decisiones

B00–B03 aceptados. B01 ya recibe mando manual 0..1000; Mandos sigue como esqueleto.
Al especificar: fijar tipos, API, estados de ausencia/error y política de recuperación;
comprobar productos, redondeo y estrechamientos para todos los tipos admitidos.
No falta información indispensable del hardware para este cálculo lógico.

## Archivos previstos

Servicios/Mandos.h/.c; tests/host/test_mandos.c; fixture cmake/verificar_enlace_b04.cmake;
CMakeLists.txt; docs/doxygen/Doxyfile; docs/agente/ y seguimiento OpenSpec.
Configuración compartida sólo si evita duplicar el límite de B01, conservando su contrato.

## Criterio de aceptación y evidencia

1. Extremos 0/escala → 0/1000; mitad del recorrido → aproximadamente 500,
   según redondeo especificado; salida monótona y siempre dentro de 0..1000.
2. Relaciones equivalentes con escalas distintas producen el mismo mando;
   cubrir, por ejemplo, escalas 1023 y 4095 sin asumir que sean las del hardware final.
3. Escala cero, lectura fuera de escala, punteros requeridos nulos y extremos de tipos
   generan errores definidos, sin overflow, división por cero ni datos viejos válidos.
4. Pruebas C host, sanitizadores, enlace ARM y regresiones B00–B03 correctos;
   Doxygen estricto sin avisos. Registrar evidencia real y límites físicos pendientes.

## Riesgos y reversión

Riesgos: atar la conversión a un ADC concreto o saturar una lectura inválida.
Mitigar con escala explícita y casos de error. Revertir sólo el diff B04,
preservando contratos aceptados, drivers y cambios ajenos; sin dependencias nuevas.

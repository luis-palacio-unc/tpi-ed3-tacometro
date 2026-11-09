# B06 — Evaluación lógica de alarma por temperatura

Estado: alcance y criterio acordados por el usuario el 2026-11-09; implementación
y commit autorizados, sin push. Umbral configurable sin valor fijo definido.

## Objetivo y origen

R07 del anteproyecto pide alarma al superar un umbral; R10/R15 separan decisión
de aplicación, servicios y acceso físico. Tras B05 y B03-R1 aceptados, proponer
la decisión de alarma en Procesamiento usando temperatura física/validez genéricas.
Driver_Alarmas aplicará posteriormente solicitudes; no debe decidir el umbral.

## Alcance y comportamiento propuesto

- Entrada: temperatura en centésimas Celsius, validez y umbral de configuración
  en las mismas unidades, entregado explícitamente. Sin conocer LM35 o ADC.
- Lectura válida: alarma activa sólo cuando temperatura>umbral; igual/inferior inactiva.
- Ausencia/error: evaluación no disponible o errónea, sin consumir dato anterior
  ni presentar esa condición como temperatura normal. Nueva lectura válida recupera.
- Decisión independiente del modo manual/automático y sin modificar mando/consigna
  o accionar el motor. Conservar las interfaces y comportamiento B01 existentes.
- Documentar las nuevas/modificadas USW con Doxygen; probar adaptación lógica de B05.

Excluye ADC/DMA, GPIO/LED/buzzer físicos, parpadeo, histéresis, enclavamiento,
timeout, parada del motor, UART, main y planificación de prioridad temporal.
La respuesta física ante fallo de lectura y la latencia prioritaria se acordarán
al integrar señalización/planificador; este bloque sólo expone el estado lógico.

## Dependencias y archivos

B00–B05 y B03-R1 aceptados. Aplicacion/Procesamiento.h/.c, pruebas host y fixture
ARM de integración, CMake/Doxyfile y seguimiento SDD. Servicios/Temperatura se usa
como contrato de temperatura/validez; Driver_Alarmas/LED/Buzzer permanecen sin accionamiento.
Sin bibliotecas nuevas. Especificación/diseño/tareas detallan el contrato acordado.

## Criterio de aceptación

1. Para varios umbrales, probar temperatura menor, igual y mayor; sólo la última activa.
2. Probar ausencia, errores, argumentos inválidos, invalidación de decisión anterior
   y recuperación, incluidos extremos int32_t sin overflow.
3. Probar independencia de modo, preservación B01 y contrato B05, sin dependencia física.
4. Host/sanitizadores, regresiones, compilación/enlace ARM y Doxygen estrictos pasan;
   registrar matriz de escenarios y separar evidencia lógica de pruebas físicas pendientes.

## Riesgos y reversión

Riesgo: confundir dato inválido con ausencia de alarma o introducir política física
no acordada. Mitigar con estados explícitos y alcance acotado. Reversión futura:
revertir sólo el commit del bloque, conservando cambios ajenos/historial.

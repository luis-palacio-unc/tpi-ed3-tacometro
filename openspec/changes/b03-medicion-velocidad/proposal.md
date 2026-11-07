# B03 — Cálculo y validez de la medición de velocidad

Estado: alcance y criterio de aceptación acordados por el usuario el 2026-11-06. Implementación autorizada; resultado aún no aceptado. Commit local autorizado, sin push.

## Objetivo y origen

Implementar Medicion_Velocidad (ESW.2.1.3) para convertir pulsos y tiempo observado
en RPM del eje que porta el disco. Origen: R02 (disco de 60 ranuras) y soporte de
R04/R09 (realimentación y visualización). R14 limita consignas, no las mediciones.

## Alcance propuesto

- Entrada: cantidad de pulsos de una ventana y duración transcurrida en microsegundos,
  obtenidas como una instantánea coherente por el futuro dispositivo de captura.
- Conversión para 60 pulsos por vuelta (un flanco por ranura), sin asumir relación
  de la reductora ni recortar lecturas a 185 RPM.
- Salida con RPM, unidades/resolución documentadas y estado de validez/error.
  Inicialmente sin medición disponible; ventana válida sin pulsos produce cero RPM.
- Validar duración nula, límites de tipos y resultado no representable; no usar
  divisiones inválidas, wrap silencioso o valores viejos como nueva medición válida.
- Documentación Doxygen por USW y pruebas C host con regresiones B00–B02.

Excluye GPIO/sensor real, temporizador/ISR/DMA, selección de ventana física, filtrado,
control PI/PID, telemetría, alimentación del motor e integración del main real.

## Dependencias y decisiones

B00–B02 aceptados. Al especificar, fijar tipos, redondeo/resolución, comportamiento
ante errores y representación de datos no disponibles. No asumir precisión o período
de actualización aún no acordados. Medir pulsos de un solo flanco y confirmar el eje
portadisco al integrar hardware; adquisición y tiempos reales permanecen pendientes.

## Archivos previstos

Servicios/Medicion_Velocidad.c/.h, tests/host/, CMake/Doxyfile y fixture ARM.
Contrato lógico de captura sólo si el diseño lo necesita; sin cambiar drivers físicos
ni archivos archivados. Utilizar runtime ya provisto por toolchain si la aritmética
requiere auxiliares, sin instalar dependencias nuevas.

## Escenarios y aceptación

1. Casos de referencia: 60 pulsos/1 s → 60 RPM, 185/1 s → 185 RPM, 200/1 s →
   200 RPM; cero pulsos en ventana positiva → cero válido.
2. Ventanas de distinta duración equivalentes dan resultados coherentes; resolución
   y error de redondeo coinciden con el contrato que se defina antes de implementar.
3. Duración cero, ausencia de datos, extremos y resultado no representable generan
   estado explícito sin comportamiento indefinido ni valores válidos inventados.
4. C host/sanitizadores, enlace ARM, regresiones y Doxygen estricto pasan; evidencia
   por escenario, sin afirmar medición física ni temporización real verificada.

## Riesgo y reversión

Riesgos: confundir límite de consigna con lectura o inventar precisión física.
Revertir sólo diff B03, conservando B00–B02 y cambios ajenos.

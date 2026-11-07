# B05 — Conversión lógica de temperatura

Estado: alcance acordado el 2026-11-07; implementación y commit autorizados, sin push. Resultado pendiente de aceptación.

## Objetivo y origen

Implementar Temperatura (ESW.2.1.4) para entregar temperatura ambiente y validez,
preparando R07 (supervisión térmica) y R09 (telemetría), con portabilidad R15.
B00–B04 están aceptados; Temperatura y LM35 aún son esqueletos.

## Alcance y comportamiento

- Entrada: muestra ADC y parámetros explícitos de conversión, referencia en µV y
  escala. No fijar resolución, VREF, canal o registros del microcontrolador.
- Modelo lineal ideal LM35, 10 mV/°C sin offset, según
  [TI, §7.3.1](https://www.ti.com/lit/ds/symlink/lm35.pdf). La aplicación recibe
  temperatura/estado, sin conocer el modelo eléctrico del sensor.
- Salida entera en centésimas de °C, redondeada al más cercano con empate arriba;
  error aritmético <=0,005 °C, separado de cuantización y error físico del sensor.
- Inicialmente sin datos. Escala/referencia nulas, muestra fuera de escala y argumentos
  inválidos generan error explícito; invalidar valor anterior y permitir recuperación.

Excluye ADC/DMA, pines, alimentación, calibración, offset/acondicionamiento para
temperaturas negativas, filtrado, frescura temporal, umbral/histéresis de alarma,
LED/buzzer, UART, regulación e integración del main.

## Dependencias y decisiones

Al especificar, fijar tipos, API, significado exacto de escala/código admisible,
representabilidad, disponibilidad/errores y recuperación antes de escribir código.
Evitar pérdidas por conversiones intermedias y overflow al redondear.
VREF real, variante/rango del LM35 y conexión deben confirmarse al integrar hardware;
no impiden verificar este modelo lógico ni se inventan como constantes de placa.

## Archivos previstos

Servicios/Temperatura.h/.c; tests/host/test_temperatura.c;
cmake/verificar_enlace_b05.cmake; CMakeLists.txt; docs/doxygen/Doxyfile;
docs/agente/ y OpenSpec. Sin alterar contratos aceptados ni drivers físicos.

## Criterio de aceptación y evidencia

1. Muestras/configuraciones equivalentes a 250 mV y 1000 mV producen 2500 y
   10000 centésimas de °C; misma tensión con escalas/referencias distintas da igual resultado.
2. Cero ideal válido se distingue de ausencia; redondeo y fronteras cumplen el contrato.
3. Escala/referencia cero, muestra inválida, NULL y extremos de tipos tienen resultados
   definidos sin división por cero, overflow ni valores antiguos presentados como válidos.
4. C host/sanitizadores, enlace ARM, regresiones B00–B04 y Doxygen estricto pasan;
   documentar USW y evidencia real, sin afirmar medición eléctrica o alarma implementadas.

## Riesgos y reversión

Riesgos: confundir referencia nominal con medida o resolución con precisión física.
Mitigar con parámetros explícitos y casos numéricos. Revertir sólo diff B05,
preservando contratos, drivers y cambios ajenos. Sin dependencias nuevas.

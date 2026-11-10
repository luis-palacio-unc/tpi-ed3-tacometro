# B10 — Selección estable de modo

Estado: alcance/criterio acordados el 2026-11-09. Implementación/commit local autorizados,
sin push; resultado pendiente de aceptación.

## Objetivo y origen

R05/R06 requieren selección exclusiva y entrada automática con nueva consigna.
Dispositivos/Selector.h asigna el antirrebote al servicio Mandos, aún sin desarrollar.
Completar esa responsabilidad bajo R10/R15 sin fijar pines/polaridad. El filtrado es
decisión propuesta; la consigna no fija su duración.

## Alcance y comportamiento propuesto

- Agregar a Mandos un contexto independiente para selector: modo lógico manual/auto,
  disponibilidad de fuente y marca temporal externas; nunca nivel GPIO concreto.
- Intervalo estable configurable al inicializar, sin valor TPI impuesto. Publicar
  el primer modo sólo tras estabilidad suficiente; cada rebote reinicia el candidato.
  Durante un cambio pendiente conservar el modo confirmado; intervalo cero acepta
  inmediatamente una muestra válida. Reiniciar exige nueva confirmación.
- Tiempo uint32_t en milisegundos con rollover modular, avances menores que 2^31 ms;
  intervalo 0..INT32_MAX. Admitir marcas iguales y detectar retrocesos/saltos fuera
  de contrato. Esto es tiempo, distinto del orden lógico de eventos B01.
- Distinguir SIN_DATOS, modo disponible y error. Fuente ausente/errónea o argumentos/
  modo/tiempo inválidos no presentan un modo anterior como vigente; exigir nueva
  estabilización tras recuperación/reinicio. Sin elegir modo por defecto ni accionar motor.
- Sin heap, espera activa o globals mutables; serialización externa. Preservar
  normalización B04. Adaptar únicamente modos confirmados a B01/B08 en pruebas;
  sin dependencia Servicios→Aplicacion. Ampliar exclusión original MA3 al aceptar B10.

Excluye GPIO/Selector físico, base de tiempos/ISR real, ADC/DMA, main, regulación,
PWM, UART y políticas nuevas de parada ante fallos. No acredita antirrebote eléctrico.

## Dependencias y archivos

B00–B09/B03-R1 aceptados. Servicios/Mandos.h/.c, pruebas host nuevas/adaptación B01/B08,
fixture ARM, CMake, documentación ESW.2.1.5/USW Doxygen y seguimiento SDD.
Sin dependencias nuevas. API, prioridad de errores y recuperación temporal exactas
se detallarán en especificación/diseño tras acuerdo; hardware sigue pendiente.

## Criterio de aceptación

1. Probar arranque sin modo, estabilidad, rebotes en ambos sentidos y límites
   justo antes/en el intervalo; cero, reinicio e instancias independientes.
2. Probar fuente ausente/fallida, NULL/modo/tiempo/configuración inválidos, marcas
   repetidas, rollover y recuperación sin publicar datos antiguos como válidos.
3. Rebotes no cambian B01; confirmar automático solicita cero y exige consigna posterior.
   Conservar contratos/pruebas B01/B04/B08 y aislamiento del hardware.
4. Pasar host/sanitizadores, regresiones, enlace ARM y Doxygen estricto; evidencia física pendiente.

## Riesgos y reversión

Confundir candidato con modo confirmado: estados y pruebas de transición explícitos.
Reversión futura por commit inverso del bloque, preservando trabajo ajeno e historial.

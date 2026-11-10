# B11 — Base de tiempos LPC1769

Estado: alcance/criterio acordados el 2026-11-09. Implementación y commit local
autorizados; sin push, resultado pendiente de aceptación.

## Objetivo y origen

Dar una referencia temporal común a los dispositivos para futuras adquisiciones
y tareas periódicas (R01/R02/R08/R09/R10/R11/R15). B10 necesita milisegundos externos;
Base_Tiempos sigue vacío. SysTick es una elección de diseño propuesta, no una
exigencia de la consigna.

## Alcance y comportamiento

- Implementar Base_Tiempos en Dispositivos/soporte: inicializar, leer milisegundos,
  detener y reiniciar, con estados/errores explícitos y sin esperas.
- Reservar SysTick para este módulo; interrupción breve que avanza un contador
  uint32_t modular por cada tick de 1 ms atendido, sin callbacks ni trabajo de servicios.
- Recibir frecuencia de CPU en Hz explícita y estable; rechazar cero, valores fuera
  de límites o que no permitan el período exacto. No asumir SystemCoreClock ni
  calibración nominal; no configurar PLL/relojes/flash ni llamar SystemInit.
- Definir lectura coherente entre ISR y lazo principal, acceso a registros,
  interrupción pendiente, parada/reinicio y estado previo a inicialización. Error
  de configuración no habilita una base inválida. Sin heap ni asignación dinámica.
- Servicios y aplicación conservan contratos independientes de SysTick/CMSIS.
  Composición temporal con Mandos únicamente en pruebas; main permanece mínimo.

Excluye planificador, demoras bloqueantes, adquisición/pulsos/ventana RPM de 1 s,
GPIO, ADC/DMA, UART, PWM, regulación y flasheo. No recupera ticks perdidos por
interrupciones demoradas; documentar límites de latencia y precisión.

## Dependencias, archivos y decisiones

B00–B10/B03-R1 aceptados. Base_Tiempos.h/.c, posible adaptador LPC1769 privado,
pruebas host/modelo de registros, fixture ARM, CMake y Doxygen/seguimiento SDD.
API y prioridad/serialización de operaciones se precisarán tras acuerdo.
Frecuencia realmente configurada y medición física pendientes; ningún valor TPI
supuesto. Consultas: [UM10360, capítulos 23/34](https://www.keil.com/dd/docs/datashts/philips/lpc17xx_um.pdf)
y [erratas NXP](https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf).

## Criterio de aceptación y evidencia

1. Host: inicialización, recarga/límites, errores, lectura sin datos, ticks,
   rollover, parada/reinicio y coherencia de estados; detectar eventos pendientes
   antiguos y verificar orden/máscaras de registros con modelo, sin afirmar ejecución MCU.
2. Probar que tiempos producidos permiten a B10 confirmar exactamente su intervalo;
   conservar regresiones y aislamiento de capas.
3. Host/sanitizadores, enlace ARM Debug/Release, vector SysTick y recursos
   inspeccionados; USW Doxygen estricto. Evidencia física separada y pendiente.

## Riesgos y reversión

Reloj declarado incorrecto o ticks omitidos falsean tiempo: precondiciones y límites
explícitos. Revertir mediante commit inverso del bloque, conservando historial y trabajo ajeno.

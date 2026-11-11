# Diseño candidato B14 — lectura Sensor_Optico

## Base y decisiones

Sensor_Optico es esqueleto ESW.2.1.10. Timer aceptado usa LPC_TIM0 y convierte TC a
milisegundos. B03 recibe pulsos/duración/factor; no se modifica ninguna de esas APIs.
La inicialización global se difiere explícitamente hasta el final.

| Decisión | Razón |
|---|---|
| TIMER1 contador externo CAP1.0/P1.18, flanco ascendente | Recurso libre de la base TIMER0 y un evento por ranura, según manual/driver. |
| SensorOptico_Read void→uint32_t | Expone conteo acumulado sin contexto/retorno ficticio ni estructura nueva. |
| Una delegación TIM_ReadTimer(LPC_TIM1) | La biblioteca ya resuelve el acceso a TC; sin registros propios. |
| Sin reset ni ventanas/IRQ/estado local | Evita perder eventos al reiniciar y conserva separación con servicio de RPM. |
| PinConfig/InitCounter/Enable sólo al integrar | Respeta alcance acordado de terminar elementos antes de configurar periféricos. |

## Operación y precondiciones

Sensor_Optico.h incluye stdint.h y documenta USW.2.1.10.1. Sensor_Optico.c incluye
lpc17xx_timer.h y su única función devuelve TIM_ReadTimer(LPC_TIM1), sin escala ni
recorte. No añadir APIs init/reset, wrapper genérico, proveedores, buffer o enum.

Recurso acordado2026-11-10. Manual tabla83/p119: P1.18 función11
CAP1.0. CTCR tabla429/p505–506: contar ascendentes por CAPn.0, CCR de ese canal000.
La biblioteca enumera TIM_CAP1_0_P1_18; configuración final usará TIM_PinConfig,
TIM_InitCounter(input TIM_CAPTURE_0,edge TIM_CTR_RISING) y TIM_Enable con TIMER1.
Usar timer exclusivo desde reset/configuración validada, sin otros escritores o
matches/capturas que alteren TC. Comprobar señal y cadencia físicamente al integrar.

El consumidor conservará primera muestra como referencia y luego restará muestras
uint32_t de la misma época. Ejemplo MAX-29→30 produce60 por resta modular. Requiere
menos de2^32 eventos entre muestras; varias vueltas completas son indistinguibles.
Reiniciar abre época nueva: descartar la muestra anterior. La ventana1s del TPI
se aplicará al integrar; el dispositivo sólo entrega eventos acumulados.

## Archivos y pruebas previstas

- Dispositivos/Sensor_Optico.h/.c: una USW documentada, código mínimo.
- tests/host/test_sensor_optico.c: fuente emulada de TIM_ReadTimer con valores
  distintos para TIMER0/TIMER1; conservar lectura acumulada y composición real B03.
  Probar0/MAX, repetición/avance, diferencias0/60/185/200, rollover y cero válido.
  No copiar implementación de registros/control de biblioteca ni crear un contador
  software de producción. La resta de ventana vive sólo en el fixture, no en firmware.
- CMakeLists.txt: target/test host con módulos Sensor_Optico y Medicion_Velocidad,
  headers originales como SYSTEM y controles estrictos existentes; fuentes base18.
- docs/doxygen/Doxyfile: INPUT Sensor_Optico.h/.c; README/docs/agente/cambio SDD.

Host/sanitizadores y regresiones; módulo ARM-O0/O2 con headers originales, header
aislado y símbolos/recursos inspeccionados; Doxygen estricto. Los tests se centran
en eventos/unidades/conservación y resultado de medición, sin revalidar el driver.
El doble no acredita conteo eléctrico ni simula CPU/latencias. No integrar main,
enlazar toda la biblioteca o ejecutar configuración. La aceptación física sigue pendiente.

Actualización de hardware del usuario: sensor XD-51, sustituyendo HC-89 mencionado
en el anteproyecto. La lectura de TC no requiere comportamiento específico del
módulo; alimentación/pinout/niveles y calidad de pulsos se verificarán al integrar.
No ampliar el código con compensaciones o filtros no pedidos.

Implementación/verificación completadas con el diseño acordado. Evidencia real y
límites por modalidad en verify-report.md; resultado pendiente de aceptación.
No se sincroniza contrato principal ni se archiva hasta aceptación del usuario.

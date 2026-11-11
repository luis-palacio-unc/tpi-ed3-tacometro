# Estado — 2026-11-11

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  HEAD de incorporación d7dc36f, origin/main62eef53; consultar Git al reanudar.
  El commit de este documento es el cierre B16. Commit local autorizado, sin push.
- B00–B16/B03-R1/B07-R1 aceptados/archivados. Sin bloque nuevo acordado.
- B16 corregido aceptado2026-11-11, implementación d7dc36f; contrato vigente
  openspec/specs/potenciometro-dma/spec.md. ADC_DMA_Pot pertenece a Potenciometro.c;
  extern interno para futuro DMA, getter0..4095 y test sin .c de soporte.
  [Cierre](../../openspec/changes/archive/2026-11-11-b16-lectura-potenciometro-dma/archive-report.md).
- Drivers90 originales a21cc1e intocables. Servicios/otros dispositivos preservados.
  Inicialización/enlace al final; main sigue siendo el lazo mínimo B00.

## Evidencia aceptada

Host/san22/22,207 casos C+oráculo; ARM13/13, objetosO0/O2/propiedad de símbolo,
Doxygen43 sin avisos. B16:3 casos de memoria, no adquisición física. Logs originales:
/tmp/ed3-b16-propiedad-pot-wve2trjj. Código/pruebas no cambiados al cerrar;
sin repetir ensayos. Checks documentales del cierre: `/tmp/ed3-cierre-b16-4gs9cvr1`.
Sin ejecución MCU/placa, simulación MCU, flasheo o publicación.

## Alcance implementado y pendientes

Procesamiento: modos/consignas/decisión térmica. Seis servicios: Medicion_Velocidad,
Temperatura, Mandos, Driver_Alarmas, Comunicacion y Driver_Motor. Dispositivos:
Timer, Motor, Sensor_Optico, LM35 (conversión), Potenciometro, Selector, LED, Buzzer,
UART, con alcance aislado de cada contrato. Soporte ADC/DMA aún sin configuración.

Próxima acción: acordar el siguiente bloque a partir de estos pendientes; no iniciarlo
automáticamente. Faltan adquisición ADC/DMA y acceso a la muestra LM35, regulador
automático, configuración de pines/reloj/periféricos, composición de servicios y main,
validación física y diagramas/informe final. Inicialización y enlace quedan al final.
Potenciómetro AD0.0/P0.23; cero inicial no indica disponibilidad. Revisar memoria
si se incorpora Sleep y secuencia ADC.2 al integrar. Sin push autorizado.

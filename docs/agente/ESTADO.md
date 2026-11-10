# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main;
  consultar HEAD. Publicado cb380bc;4260249/92b4d62 y cierre documental locales.
- B00–B12/B03-R1 aceptados/archivados. B12 corregido aceptado2026-11-10: contrato
  openspec/specs/dispositivos-gpio/spec.md; archivo2026-11-10-b12-dispositivos-gpio.
- Selector_Read bool(void) P2.6 alto; LED_Write/Buzzer_Write void(bool), P0.22 bajo/
  P2.7 alto, sin contexto/retorno. Doxygen vigente. Biblioteca90 originales intactos.
- Activo propuesto: b07-r1-salidas-sin-contexto, B07-R1. Alcance pendiente de acuerdo;
  sin implementación/pruebas de revisión. Propuesta/spec/diseño/tareas preparados.
- Driver_Alarmas aún bool(void*,bool)/contextos/5 estados. B07-R1 propone void(bool),
  dos callbacks sin contextos y sólo ACEPTADA/ERROR_ARGUMENTO (despacho/validación).
  Servicio seguirá sin dependencia del MCU. Conexión B12 pendiente hasta implementar.
- Inicialización/enlace global/main/hardware para el final. No modificar Drivers;
  b12-integracion-arranque-timer deferred. Sin push autorizado.

## Evidencia aceptada y cierre

B12 (92b4d62): host/san18/18,186 casos C y oráculo Python;7 casos GPIO directos.
ARM13/13 y salidasO0/O2 compiladas, no ejecutadas. Doxygen38 sin avisos.
Negativos de polaridad detectados, Drivers/B06/B07 conservados; sin ensayo físico.
[Evidencia B12](../../openspec/changes/archive/2026-11-10-b12-dispositivos-gpio/verify-report.md).

Cierre actual sólo documental: sincronización normativa DG1–DG3/S1–S4, archivo,
hashes/enlaces/estructura/diff. No se repiten pruebas de firmware ni se atribuyen
las previas a B07-R1. Logs externos: /tmp/ed3-cierre-b12-propuesta-b07r1-eh97fprw.

## Próxima acción y límites

Acordar [B07-R1](../../openspec/changes/b07-r1-salidas-sin-contexto/proposal.md).
Después implementar/migrar consumidores, verificar y commit local; no comenzar aún.
Contrato salidas-alarma sigue vigente hasta aceptación de la revisión. No casts/
adaptadores temporales. GPIO exige dirección/mux/polarización/FIOMASK previos y
acceso serializado. Revisión física/cableado/configuración/enlace pendientes.
Quedan Motor, Sensor_Optico, Potenciometro, UART, ADC-DMA/adquisición LM35 y
regulación/integración. Diagramas del informe al completar todos los ESW.

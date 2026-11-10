# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`,
  main, consultar HEAD. Base e95205e; publicado cb380bc; commits locales sin push.
- B00–B12/B03-R1/B07-R1 aceptados/archivados. Activo b13-dispositivo-uart:
  alcance/UART0 autorizados2026-11-10, implementado/verificado; resultado sin aceptar.
- UART_Read/Write delegan una vez en Receive/Send originales con NONE_BLOCKING;
  retornan bytes reales, NULL/cero sin accesos. Dos USW16.1/.2 documentadas Doxygen.
- TX P0.2/RX P0.3 función01 asignados por manual tabla80/p118 inspeccionada.
  UART_BAUDRATE en UART.h, default115200, editable/override -D; guardas de entero.
  Parámetro preparado, no aplicado; no se configura pinmux/baudios ni init aquí.
- Drivers90 originales a21cc1e, servicios/aplicación/otros dispositivos/main y
  archivo/contratos aceptados intactos. Commit local al terminar; sin push/sincronizar/
  archivar hasta aceptación/orden correspondientes. No iniciar bloque siguiente.

## Verificación B13 realmente ejecutada

| Modalidad | Resultado |
|---|---|
| Host/san |19/19 cada uno,194 casos C+oráculo Python; sin hallazgos |
| UART individual |11 casos: vacíos/RX/TX binario/canarios/parciales/continuación/recuperación |
| ARM |Build/controles13/13, objetosUART-O0/O2; sin ejecución MCU |
| Parámetro |Default115200, overrides9600/57600/230400 C11/C++17;0/-1/>uint32 rechazados; móduloARM9600 compila |
| Doxygen |40 funciones únicas HTML/XML sin avisos,2 nuevas |
| Negativos externos |Detectan progresoTX ficticio, excesoRX y modo bloqueante |
| Recursos propios |.text104-O0/48-O2,.data/.bss0; marcos16/16-O0,0/0-O2 sin driver/caller/IRQ |
| Preservación |252 archivos protegidos intactos,90 Drivers originales; fuente firmware sigue18 objetos |

[Informe](../../openspec/changes/b13-dispositivo-uart/verify-report.md).
Logs externos: /tmp/ed3-b13-uart-ku1mtzv7. Manual renderizado legible pese a avisos de caché fontconfig.
Host usa doble de API, no ejecuta driver/CPU/electricidad. Compilación ARM no
acredita enlace de biblioteca completa ni recepción PC. Sin instalación/flasheo/push.

## Próxima acción y límites

Revisar/aceptar B13 antes de sincronizar/archivar. Aplicación/main, configuración de
periféricos, UART_PinConfig/Init/baudios/formato/reloj/FIFO/DLAB, cableado/conversor y
ensayos físicos quedan para el final. Serializar RX/TX, sin otros consumidores ISR/DMA.
Diagnóstico de línea/cadencia/colas no implementados en B13. Quedan Motor,
Sensor_Optico, Potenciometro, ADC-DMA/LM35 y regulación/integración. No retomar
b12-integracion-arranque-timer (deferred); diagramas del informe al completar ESW.

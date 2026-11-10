# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`,
  main, consultar HEAD. Publicado cb380bc; fe55aa9 y cierres locales, sin push.
- B00–B12/B03-R1/B07-R1 aceptados/archivados. B07-R1 cerrado2026-11-10:
  contrato salidas-alarma sincronizado; nota de composición dispositivos-gpio actualizada.
- Driver_Alarmas: dos funciones void(bool), sin contextos, ACEPTADA/ERROR_ARGUMENTO,
  validación previa y LED→buzzer. Composición B12 probada en host; aplicación/main
  explícitamente pendientes para el final por instrucción del usuario.
- Activo propuesto b13-dispositivo-uart (B13), sin acuerdo/implementación/pruebas:
  UART_Read/UART_Write, buffers externos y progreso uint32_t, driver NONE_BLOCKING.
  UART0 propuesto. Propuesta/spec/diseño/tareas preparados; no implementar aún.
- Biblioteca90 originales intactos, no modificar Drivers. Inicialización/enlace global/
  configuración de periféricos/uso desde main y pruebas físicas se dejan para el final.
  Sin push autorizado; no retomar b12-integracion-arranque-timer, deferred.

## Evidencia aceptada y cierre actual

B07-R1 fe55aa9: host/san18/18,183 casos C y oráculo Python;8 servicio,4 térmicos,
8 GPIO. ARM13/13 y fixture Release; objetos/headers aislados, Doxygen38 sin avisos.
Negativos detectan orden/omisión de buzzer/acción antes de validar; Drivers/B06/B12 intactos.
[Informe](../../openspec/changes/archive/2026-11-10-b07-r1-salidas-sin-contexto/verify-report.md).

Cierre sólo documental: correspondencia de requisitos, estructura/enlaces/hashes/diff;
no se repiten pruebas de firmware ni se acredita B13. Fuentes/periféricos intactos.
Logs externos: /tmp/ed3-cierre-b07r1-propuesta-b13-upsumg8j.

## Próxima acción y pendientes

Acordar [B13 UART](../../openspec/changes/b13-dispositivo-uart/proposal.md), incluido
recurso UART0, antes de implementar. USW previstas .2.1.16.1/.2. Uso físico exige
UART/FIFO/pines/reloj configurados, DLAB0 y acceso exclusivo/serializado con ISR/DMA.
Baudios/formato, cableado/conversor, diagnóstico de línea, cola/cadencia e integración
con Comunicacion/main quedan pendientes; no suponer USB serie por CMSIS-DAP.
Quedan además Motor, Sensor_Optico, Potenciometro, ADC-DMA/LM35 y regulación.
Diagramas completos del informe al terminar todos los ESW.

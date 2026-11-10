# Estado — 2026-11-10

## Punto actual

- Repositorio `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`;
  main, consultar HEAD. Base publicada cb380bc; planificación local previa preservada.
- B00–B11/B03-R1 aceptados/archivados. Activo b12-dispositivos-gpio: alcance autorizado
  el2026-11-10, implementado/verificado, resultado pendiente de aceptación.
- Selector_Read/P2.6 y Buzzer_Write/P2.7 activos por alto, LED_Write/P0.22 por bajo.
  Sólo GPIO_ReadValue/SetPinState originales; tres USW documentadas con Doxygen.
  Salidas compatibles con B07; contextos ignorados/NULL, true sólo acredita delegación.
- Prohibido modificar Drivers:90 archivos originales a21cc1e intactos. Dispositivos
  accede al hardware directamente; servicios y aplicación aceptados sin cambios.
- Inicialización de periféricos, enlace global y main diferidos al final por usuario;
  b12-integracion-arranque-timer sigue deferred. No avanzar al siguiente bloque.
- Commit local autorizado al cierre: `feat: implementar Selector LED y Buzzer con GPIO original`.
  Sin push/sincronización/archivo hasta orden y aceptación correspondientes.

## Verificación ejecutada

| Modalidad | Resultado |
|---|---|
| RED→GREEN | Interfaces ausentes detectadas;9 casos nuevos pasan |
| Host/sanitizadores |18/18 cada uno,188 casos C + oráculo Python, sin hallazgos |
| Regresiones |Velocidad11199 ternas, codec5096 pares, selector4096 muestras/400 vueltas |
| ARM |Debug13/13; módulosO0/O2 compilados con headers originales, sin ejecución |
| Doxygen |38 funciones propias únicas,3 nuevas, HTML/XML sin avisos |
| Negativos |Detecta selector usando bit7 y buzzer con polaridad invertida en copias externas |
| Recursos propios |.text104-O0/52-O2, .data/.bss0; marcos8/16/16-O0,8/8/8-O2 sin driver/IRQ |
| Preservación |220 archivos protegidos intactos,90 Drivers originales; main/servicios/Timer/LM35/archivo aceptado conservados |

Informe: openspec/changes/b12-dispositivos-gpio/verify-report.md.
Logs/snapshot externos: /tmp/ed3-b12-gpio-implementacion-m6evumxd.
No simulación MCU, ejecución ARM/placa, programación o instalación. El GPIO host
es doble de API, no driver físico ni simulación de CPU. No declarar tiempos/estado
eléctrico por compilación; firmware mínimo descarta por GC módulos no llamados.

## Próxima acción y límites

Revisar/aceptar B12 antes de sincronizar/archivar/publicar. GPIO requiere configuración
previa: dirección, multiplexado, polarización y FIOMASK; acceso serializado privilegiado.
LED según esquema OM13085 revD, revisión física/cableado pendientes. Antirrebote y
modo se conservan en Mandos; condición térmica en aplicación/servicios existentes.
Quedan Motor, Sensor_Optico, Potenciometro, UART y soporte ADC-DMA; LM35 tiene
conversión numérica, adquisición pendiente. Integración/inicialización/enlace finales.

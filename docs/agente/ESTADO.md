# Estado — 2026-11-10

## Punto actual

- Repositorio `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`;
  main, consultar HEAD. Publicado cb380bc; B12 original4260249 y corrección locales.
- B00–B11/B03-R1 aceptados/archivados. Activo b12-dispositivos-gpio: alcance y
  corrección autorizados, implementado/verificado; resultado pendiente de aceptación.
- Selector_Read/P2.6 y Buzzer_Write/P2.7 activos altos; LED_Write/P0.22 bajo.
  Salidas corregidas: void(bool activar), sin contexto ni retorno. Doxygen actualizado.
- Driver_Alarmas/B07 permanece intacto; su bool(void*,bool) no admite estas salidas.
  Revisión separada pendiente por pedido del usuario; sin adaptadores/casts temporales.
- Drivers:90 originales a21cc1e intactos. Dispositivos usa biblioteca directamente;
  servicios/aplicación aceptados conservados. Inicialización/enlace/main para el final.
- Commit normal local autorizado al cierre de corrección; sin push/sincronizar/archivar
  hasta orden y aceptación correspondientes. No avanzar otro bloque.

## Verificación de la corrección ejecutada

| Modalidad | Resultado |
|---|---|
| Host/sanitizadores |18/18 cada uno;186 casos C y oráculo Python, sin hallazgos |
| B12 directo |7 casos: bits/polaridad/aislamiento/repetición; composición B07 pendiente |
| ARM |Debug13/13; dos salidasO0/O2 compiladas con headers originales, sin ejecución |
| Doxygen |38 funciones únicas HTML/XML sin avisos; salidas void(bool), sin @return |
| Negativos externos |Inversión de polaridad LED/buzzer detectada |
| Recursos de salidas |.text36/30-O0 y16/12-O2; .data/.bss0; pila propia16/16 y0/0, excluye driver/IRQ |
| Preservación |224 archivos protegidos intactos,90 Drivers originales; B07/archivo aceptado sin cambios |

Informe: openspec/changes/b12-dispositivos-gpio/verify-report.md.
Logs externos: /tmp/ed3-b12-salidas-simples-ns5j3oo1.
El doble GPIO no ejecuta biblioteca/CPU ni acredita estado eléctrico. Sin ensayo en
placa, simulación MCU, flasheo, instalación o publicación. Firmware mínimo sin uso
de módulos GPIO; no validar enlace global por compilación de objetos.

## Próxima acción y límites

Revisar/aceptar B12 corregido y acordar revisión separada de Driver_Alarmas/B07 para
quitar contexto y retorno de proveedores; conexión real pendiente hasta entonces.
GPIO exige configuración previa de dirección, mux, polarización y FIOMASK, acceso
privilegiado serializado y configuración estable; revisión física/cableado pendientes.
Antirrebote/modo siguen en Mandos; decisión térmica en aplicación/servicios.
Quedan Motor, Sensor_Optico, Potenciometro, UART y soporte ADC-DMA; adquisición LM35
pendiente. b12-integracion-arranque-timer sigue deferred por decisión del usuario.

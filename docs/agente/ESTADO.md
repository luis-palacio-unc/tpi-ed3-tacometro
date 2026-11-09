# Estado — 2026-11-09

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD con git log -1.
- B00–B05 y B03-R1 aceptados/archivados. Revisión B03-R1 aceptada por usuario el
  2026-11-09, contrato de medición sincronizado (cuatro requisitos, diez escenarios).
- Implementación B03-R1: 8383e56e8f323a036d83849257cf6d1b2474b029.
  Cierre/propuesta en commit `docs: aceptar B03-R1 y proponer B06`.
- Base remota conocida origin/main=a11a97c1c8617fdc42007afa78d071e83b553512.
  Revisión y cierre permanecen locales; no hay nueva autorización de push.
- Cambio activo b06-logica-alarma-temperatura: sólo propuesta, alcance pendiente de acuerdo.
  Usuario confirmó umbral como parámetro de configuración, sin valor fijo.

## Evidencia aceptada

- B03-R1, ejecutada el 2026-11-07: 102 casos C (23 B03), 11199 ternas de oráculo
  exacto y sanitizadores PASS. ARM Debug 8/8, B03 Release 1/1 y Doxygen estricto.
- Aislamiento sin configuración/dispositivo, prioridades/recuperación y extremos
  verificados; recursos/pila y matriz S1–S10 en informe archivado B03-R1.
- Cierre 2026-11-09 sólo documental: fusión por requisito, hashes de código/historial,
  YAML, enlaces y diff. No se ejecutan ni declaran pruebas nuevas de firmware.
- Logs cierre: /tmp/ed3-cierre-b03-r1-20261109-7nh1jdsb; logs originales en informe archivado.
- Sin ejecución ARM, simulación o prueba física. Sin instalación/flasheo ni cambios MCU.

## Decisiones y pendientes

Servicios independientes de modelo físico: Temperatura en Celsius/estado, velocidad
con factor pulsos/vuelta explícito. TPI usa 60 en configuración externa al servicio.
B06 propuesto: evaluar temperatura>umbral en Procesamiento y distinguir ausencia/error;
no cambiar comportamiento B01 ni accionar dispositivos. Driver_Alarmas aplicará
solicitudes en una integración posterior. Respuesta física a fallo, prioridad temporal,
ADC/DMA, captura, UART, PWM/control y main siguen pendientes.
CMSIS-DAP integrado; revisión física PCB/silicio, pines/VREF/montaje pendientes.

## Próxima acción

Acordar alcance/aceptación B06. Después especificación, diseño, tareas e implementación
por orden del usuario. No iniciar código B06 ni publicar commits sin autorización.

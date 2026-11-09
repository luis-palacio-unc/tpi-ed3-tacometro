# Estado — 2026-11-09

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD y remoto con Git al incorporarse.
- B00–B06 y B03-R1 aceptados/archivados. B06 aceptado el 2026-11-09;
  contrato vigente alarma-temperatura sincronizado (cuatro requisitos/ocho escenarios).
- B06 implementación 47ae9b5; cierre documental c09895576e129c12d32d7a5fc473293fbbd5ab6d.
- Usuario autorizó publicar commits pendientes y cierre B06; esta actualización
  incluye su propuesta B07 solicitada, sin código nuevo. Antes de cerrar la orden,
  comprobar publicación con HEAD==origin/main y referencia remota. Log: /tmp/ed3-cierre-b06-20261109-gqz_d7bw/publicacion.log.
- Cambio activo b07-coordinacion-salidas-alarma: sólo propuesta, alcance pendiente de
  acuerdo. No hay autorización de implementación ni de publicación futura de B07.

## Evidencia aceptada

- B06: 115 casos C (13 B06), 11199 ternas B03-R1 y sanitizadores PASS;
  ARM Debug 9/9, B06 Release 1/1, Doxygen estricto 22 funciones/USW.
- Matriz AT1–AT4/S1–S8 y recursos en informe archivado B06.
- Cierre/propuesta sólo documentales: contrato exacto, preservación de fuentes/historial,
  manifiesto, YAML/enlaces y diff. No se repiten ni declaran pruebas nuevas de firmware.
- Sin ejecución ARM, simulación o hardware; sin instalaciones/flasheo/cambios MCU.
- Logs de cierre/propuesta/publicación: /tmp/ed3-cierre-b06-20261109-gqz_d7bw; logs B06 en informe archivado.

## Pendientes y próxima acción

Acordar alcance/aceptación B07 propuesto: solicitudes lógicas hacia salidas LED/buzzer
reemplazables, verificables con emulación. No decidir umbral en servicios ni introducir
Servicios→Aplicacion. Fallos/ausencia de B06 no se interpretarían como orden de apagado.
GPIO/pines/polaridad, señales reales, respuesta física ante lectura fallida y prioridad
temporal siguen pendientes; también adquisición, UART, PWM/control y main.
CMSIS-DAP integrado; revisión física PCB/silicio y montaje/pines/VREF sin verificar.
No implementar B07 hasta acuerdo; no inferir permisos para futuras publicaciones.

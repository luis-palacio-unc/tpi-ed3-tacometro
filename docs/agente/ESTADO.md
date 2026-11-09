# Estado — 2026-11-09

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD/remoto con Git al incorporarse.
- B00–B07 y B03-R1 aceptados/archivados. B07 aceptado el 2026-11-09;
  contrato salidas-alarma sincronizado (cuatro requisitos/ocho escenarios).
- Implementación B07: 92bfceadd7c6e34e3ea841007a3e996d220d296f.
- Cierre/propuesta documental en commit `docs: aceptar B07 y proponer B08`.
  Usuario autorizó push de pendientes/cierre y pidió propuesta B08. Antes de cerrar
  esta orden, comprobar HEAD==origin/main==remoto; resultado en /tmp/ed3-cierre-b07-20261109-298bmkta/publicacion.log.
- Cambio activo b08-mando-motor-portable: sólo propuesta, alcance pendiente de acuerdo;
  implementación y publicación futura no autorizadas.

## Evidencia aceptada

- B07: 131 casos C (12 servicio + cuatro integración), 11199 ternas B03-R1 y
  sanitizadores PASS; ARM Debug 10/10, B07 Release 1/1 y Doxygen estricto 23 funciones.
- Matriz SA1–SA4/S1–S8 y recursos en informe archivado B07.
- Cierre/propuesta sólo documentales: contrato exacto, hashes de fuentes/historial,
  manifiesto, YAML/enlaces y diff. No se repiten ni declaran pruebas nuevas de firmware.
- Sin ejecución ARM, simulación o hardware; sin flasheo/instalación/cambios MCU.
- Logs de cierre/propuesta/publicación: /tmp/ed3-cierre-b07-20261109-298bmkta; logs B07 en informe archivado.

## Pendientes y próxima acción

Acordar alcance/aceptación B08 propuesto: Driver_Motor con mando 0..1000 por mil,
solicitud de cero y salida reemplazable; no confundir consigna RPM con PWM.
Control automático, PWM/pines/frecuencia/potencia reales, adquisición, UART y main
siguen pendientes; también GPIO/polaridad/tiempos reales de alarmas y patrón físico de fallo.
CMSIS-DAP integrado; revisión PCB/silicio y montaje/pines/VREF sin verificar.
Documentación final: diagramas de secuencia breves por servicios para el informe,
después de implementar todos los ESW, como pidió el usuario. No elaborarlos todavía.
No implementar B08 ni publicar trabajo futuro sin acuerdo.

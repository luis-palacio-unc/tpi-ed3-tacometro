# Estado — 2026-11-07

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; comprobar HEAD con `git log -1`.
- B00–B05 aceptados/archivados; temperatura corregida aceptada el 2026-11-07.
- B05 inicial fcd4392, corrección a1f3b46; cierre documental en commit con asunto
  `docs: aceptar y archivar B05 corregido`. Contrato vigente de temperatura sincronizado.
- Publicación de commits pendientes y cierre B05 autorizada en esta orden.
  Verificar resultado remoto con Git al terminar push; no inferir publicación futura.
- Revisión B03 autorizada por el usuario: quitar 60 pulsos/vuelta fijos del servicio,
  crear cambio SDD independiente; no editar historia aceptada.
- main mínimo; sin adquisición ni integración física.

## Evidencia aceptada

- B05: 94 casos host/sanitizadores; CTest ARM Debug 8/8, dos enlaces B05 Release;
  Doxygen estricto, 21 funciones. Informes en archivo B05, no hay fallos del alcance.
- Temperatura genérico compila/enlaza sin LM35; dispositivo convierte ADC→Celsius.
- Cierre documental: cuerpo normativo idéntico, fuentes/build/pruebas e historial
  previo preservados. No repite pruebas ni acredita nuevos comportamientos.
- No hubo ejecución ARM, simulación o prueba física. Sin instalaciones/flasheo.
- Logs cierre: /tmp/ed3-cierre-b05-20261107-ylmn3frc; logs B05 en informe archivado.

## Pendientes y próxima acción

Publicar cierre B05 y abordar revisión B03 con requisitos/especificación/diseño,
pruebas numéricas y de error, ARM, recursos y Doxygen. Mantener contrato vigente de
B03 hasta aceptación del cambio nuevo y conservar su archivo inmutable.
ADC/DMA, captura física, pines/VREF/montaje, UART, PWM/control, alarma y main siguen
pendientes. CMSIS-DAP integrado; revisión PCB/silicio sin verificar.

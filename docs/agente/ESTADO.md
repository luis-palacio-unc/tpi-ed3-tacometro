# Estado — 2026-11-09

## Punto actual

- Repositorio: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD/remoto con Git al incorporarse.
- B00–B08 y B03-R1 aceptados/archivados. B08 aceptado el 2026-11-09;
  contrato mando-motor sincronizado (cuatro requisitos/ocho escenarios).
- Implementación B08: 26374b2ceb2aaf49fb6f352e22a69f6b658751d1. Cierre/propuesta en commit
  `docs: aceptar B08 y proponer B09`. Publicación autorizada en esta orden;
  cotejo HEAD==origin/main==remoto en /tmp/ed3-cierre-b08-20261109-hyxg3jtv/publicacion.log al completarla.
- Cambio activo b09-codificacion-telemetria: sólo propuesta; alcance/formato/criterio
  pendientes de acuerdo, sin implementación ni publicación futura autorizadas.

## Evidencia aceptada y cierre

- B08: host/san 13/13, 146 C (10 servicio + cinco integración) y 11199 ternas;
  ARM Debug 11/11 y fixture B08 Release; Doxygen estricto 25 funciones/USW.
- Matriz MM1–MM4/S1–S8, recursos e incidente san resuelto en informe archivado B08.
- Cierre documental: igualdad del contrato, hashes/manifiesto, YAML/enlaces y diff;
  sin cambios de firmware ni pruebas nuevas de código. Logs externos: /tmp/ed3-cierre-b08-20261109-hyxg3jtv.
- Sin ejecución ARM, simulación o placa; sin flasheo/instalación/cambios sensibles MCU.

## Pendientes y próxima acción

Acordar B09 propuesto: codificación ASCII de RPM/temperatura/estado con validez y
buffer acotado. El formato es propuesta de diseño, no requisito original aceptado.
Recepción B02 preservada; ampliar su exclusión de telemetría al aceptar B09.
UART/envío/periodicidad, adquisición, PWM/potencia/regulación/main físicos pendientes.
CMSIS-DAP integrado; revisión PCB/silicio, montaje/pines/VREF sin verificar.
Diagramas finales después de completar los ESW, según CONTEXTO.
No implementar B09 ni publicar trabajo futuro sin acuerdo.

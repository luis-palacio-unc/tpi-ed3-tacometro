# Estado — 2026-11-09

## Punto actual

- Repositorio: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD/remoto al incorporarse.
- B00–B09 y B03-R1 aceptados/archivados. B09 aceptado el 2026-11-09;
  comunicación sincronizada: cuatro requisitos agregados, B02-R5 modificado;
  B02-R1–R4 intactos, total nueve requisitos/17 escenarios.
- Implementación B09: 84c51e296396f7c730ce4b18c2efc4c2cc1e57c4. Cierre/propuesta en commit
  `docs: aceptar B09 y proponer B10`. Publicación autorizada en esta orden;
  cotejo HEAD==origin/main==remoto en /tmp/ed3-cierre-b09-20261109-pufv9luj/publicacion.log al completarla.
- Cambio activo b10-seleccion-estable-modo: sólo propuesta; alcance/criterio
  pendientes de acuerdo, sin implementación ni publicación futura autorizadas.

## Evidencia aceptada y cierre

- B09: host/san 15/15, 161 C (11 codec + cuatro integración), 5096 pares codec y
  11199 ternas B03-R1; ARM Debug 12/12 y fixture B09 Release; Doxygen 29 funciones.
- Matriz, recursos y límites en informe archivado B09.
- Cierre documental: merge por requisitos, hashes/manifiesto, YAML/enlaces y diff;
  sin cambios de firmware ni pruebas nuevas. Logs externos: /tmp/ed3-cierre-b09-20261109-pufv9luj.
- Sin ejecución ARM, simulador o placa; sin flasheo/instalación/cambios sensibles MCU.

## Pendientes y próxima acción

Acordar B10 propuesto: completar selección estable de modo en Mandos con intervalo
configurable, marcas temporales/muestras externas y estados explícitos. Tiempo
distinto del orden de eventos B01. Normalización B04 y aplicación preservadas.
GPIO/polaridad, base de tiempos, ADC/DMA/captura, UART/TX, PWM/potencia/regulación y
main físicos pendientes. CMSIS-DAP integrado; revisión PCB/silicio/pines/VREF sin verificar.
Diagramas finales después de completar ESW, según CONTEXTO.
No implementar B10 ni publicar trabajo futuro sin acuerdo.

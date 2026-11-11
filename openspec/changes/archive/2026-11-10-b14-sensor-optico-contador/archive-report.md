# Archivo B14 — 2026-11-10

El usuario solicita cerrar B14, publicar los commits pendientes y proponer el
siguiente bloque. Se acepta el resultado de lectura acordado; implementación
3fa08ea, 10/10 tareas completas. Configuración y prueba física siguen pendientes.

## Sincronización

Creado openspec/specs/sensor-optico/spec.md con SO1–SO4 y S1–S5 idénticos al
candidato; sólo título/procedencia reflejan aceptación. Estado actualizado antes
de mover el cambio. Propuesta/diseño/spec/tareas/verify-report conservan redacción
histórica, incluidas menciones a aceptación pendiente; este informe y state.yaml
registran el cierre posterior. Otros contratos y archivos históricos intactos.

## Evidencia aceptada y límites

Host/sanitizadores 20/20, 200 casos C y oráculo Python; seis casos B14.
ARM 13/13 y objetos O0/O2 compilados; Doxygen HTML/XML 41 funciones sin avisos.
Biblioteca original intacta. Evidencia real en verify-report.md y
/tmp/ed3-b14-optico-ibtde439; no se repiten ni declaran pruebas nuevas de firmware
en este cierre documental.

SensorOptico_Read entrega conteo acumulado uint32_t por TIM_ReadTimer(LPC_TIM1),
sin reset/ventanas/RPM. XD-51 confirmado por usuario. CAP1.0/P1.18 es entrada del
contador externo de TIMER1; no se habilita captura de timestamps en ese canal.
PinConfig/InitCounter/Enable, compatibilidad eléctrica, ventana de 1 s y conexión
a servicios/main quedan para integración final. TIMER0/base, B03 y Drivers intactos.
No ejecución ARM/placa, simulación MCU, instalación ni flasheo.

Publicación autorizada de ocho commits pendientes más este cierre/propuesta.
Este informe registra la autorización; el resultado de push se comprueba en Git.
No merge, rebase o reescritura autorizados. B15 se presenta como candidato, sin
código/pruebas nuevos y sin inicializar periféricos. Revisar/acordar antes de implementar.

Cierre: igualdad normativa, estructura/enlaces, YAML y SHA256 de fuentes,
biblioteca y archivos previos; diff completo. Logs externos: /tmp/ed3-cierre-b14-propuesta-b15-uejb662y.

## Manifest SHA256 (sin este informe)

- `design.md`: `c36ab704db25585d63ff2cd6323a84a568354bc84e497aa0e94e8b4db2d979dc`
- `exploration.md`: `6f6122e871a57974a71b2f0f49c59b9918ca5a34d61eb635790f777ef0b21f3e`
- `proposal.md`: `15a62c5977872015ab58230cb58d9a311c11b3766a96bc97604452df45112fe7`
- `specs/sensor-optico/spec.md`: `8ec54ece08feac77680c1db36fdda283c45976fef97ad0685622ff63be60efe7`
- `state.yaml`: `9e0f621060f9ef16f265ae200bad186d68c4ea20e965364672ebf7fc200820b2`
- `tasks.md`: `804ea84ab1f18aa6ab7216731b013795b6d8b03a3b7a67c8378ed862d51a65eb`
- `verify-report.md`: `c9bb796161e03eb3d5c02c916190ca4fc6b97117712a7b1b46048dafe45de31d`

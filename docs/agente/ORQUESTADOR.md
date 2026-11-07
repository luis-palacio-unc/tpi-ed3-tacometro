# Procedimiento de trabajo SDD

## Incorporación

1. Leer ESTADO antes que otros documentos; comprobar proyecto, Git, rama y HEAD.
2. Leer AGENTS aplicables y revisar cambios locales. Si no existe Git o la rama no
   está acordada, resolverlo antes de implementar; no crear commits por inferencia.
3. Cargar CONTEXTO, requisitos y sólo las specs/fuentes del bloque actual.
4. Contrastar consigna y aclaraciones con la implementación. Si discrepan, registrar
   fuentes y resolver la decisión antes de cambiar el comportamiento afectado.

## Selección y especificación

5. Proponer un bloque Bxx con origen, objetivo, alcance/exclusiones, dependencias,
   decisiones pendientes, archivos, entradas/salidas/errores y aceptación verificable.
6. Esperar acuerdo del bloque. Una orden «seguí con Bxx» autoriza completarlo dentro
   del alcance acordado, no iniciar bloques posteriores.
7. En `openspec/changes/<cambio>/`, preparar proposal.md, specs/<area>/spec.md,
   design.md, tasks.md y state.yaml. Usar skills SDD aplicables. Las specs de cambio
   son candidatas; no actualizar contratos vigentes antes de la aceptación.

## Implementación y verificación

8. Implementar la menor solución que satisface el contrato. Para errores de
   comportamiento, reproducir con una prueba cuando sea pertinente.
9. Verificar límites/conversiones C, recursos, estado compartido/ISR, temporización,
   inicialización, fallos y recuperación según el alcance del bloque.
10. Ejecutar las verificaciones aplicables y registrar comando, entorno, resultado y
    evidencia en verify-report.md. Distinguir host, ARM, simulación y hardware.
    Hardware faltante deja aceptación física pendiente, no impide trabajo independiente.
11. Revisar diff completo, cambios ajenos, hooks y datos sensibles. No desactivar
    controles para obtener verde. Logs extensos fuera del árbol versionado.

## Cierre

12. Actualizar tasks.md, state.yaml, ESTADO y MAPA_REQUISITOS con evidencia real.
    Actualizar CONTEXTO sólo ante decisiones duraderas, sin transcripciones.
13. Si están autorizados commits del bloque, acordar firma, conservar hooks y crear
    commits pequeños tras controles. Push/merge/reescritura requieren permiso aparte.
14. Distinguir implementado, verificado y aceptado. Sin aceptación del alcance,
    no sincronizar specs principales ni archivar. Una vez aceptado, sincronizar y
    mover a `archive/AAAA-MM-DD-<cambio>/`; no editar historial archivado.
15. Informar resultados y limitaciones; dejar una próxima acción concreta y detenerse.

Este archivo es un procedimiento escrito. No ejecuta servicios ni trabajo autónomo.
No instalar dependencias, usar sudo, flashear o cambiar configuraciones sensibles
sin autorización vigente para ese alcance.

# Cierre B17 — 2026-11-11

Usuario acepta el resultado 8f1793f36be1c73e63d8e8f0bbc9ca26852d244a y autoriza sincronizar,archivar,
crear commit local y hacer push de main. La reauditoría externa local
B17-implementacion-r2.md declara APROBAR,sin hallazgos nuevos; H1/H2 resueltos.
[Resumen portable](review-notes.md),sin enlazar ni publicar el harness.

## Sincronización y archivo

Nuevo [contrato vigente](../../../specs/regulacion-velocidad/spec.md):
RV1–RV6 y S1–S10 completos, incluido m1. Cuerpo normativo idéntico al aceptado;
sólo cambian cabecera de estado y enlaces hacia evidencia/diseño archivados.
SHA-256 del cuerpo posterior a Requirements:
1d902e71508ec16812dd85aae712b28bd3e6129baf256d611bb1b8e0e6404076.

Cambio movido a openspec/changes/archive/2026-11-11-b17-regulador-velocidad; carpeta activa retirada.21/21 tareas completas.
Ocho artefactos:proposal,design,tasks,state,spec,verify-report,review-notes y
archive-report. Design/tasks/spec/verify-report conservan evidencia histórica
de implementación; los pendientes de auditoría/aceptación que contienen quedan
resueltos por este cierre. No se edita ningún cambio archivado previamente.

ESTADO/CONTEXTO/MAPA/config y README actualizados. Código,pruebas,build,Drivers,
main/servicios/dispositivos y contratos vigentes anteriores preservados.
La publicación autorizada incluye los commits pendientes y el commit de cierre;
evidencia local de Git en build/cierre-b17-2026-11-11.

## Evidencia aceptada y controles de cierre

Ejecución propia de8f1793f, documentada en [verify-report](verify-report.md):
host/san33/33 cada uno,226 casos C por modalidad,133758 pasos Fraction exactos,
8 configuraciones,25 probes y cinco mutantes detectados (copia correcta pasa).
ARM15/15,Doxygen46 sin avisos,instancia40bytes. Planta200RPM al100%,polo3/4 y
retardo1paso:establecimiento14/14,picos99,6/185,2RPM. No es sintonía física.
Logs/copias/todas las invocaciones en build/verificacion-b17-h1-h2-2026-11-11.

Claude informó reproducción independiente del verificador y detección de sus14
mutantes; se distingue de las cinco mutaciones propias. Es evidencia externa.
No se atribuyen esas ejecuciones al agente que cierra el cambio.

Este cierre sólo ejecuta controles documentales: igualdad del cuerpo normativo,
hashes de preservación,existencia de artefactos,estados YAML,enlaces y diff.
Fuentes actuales cotejadas con manifiestos de las siete fases aceptadas; no se
repiten compilación,sanitizadores,simulación de planta o Doxygen sin cambios de código.
Evidencia de estos controles:build/cierre-b17-2026-11-11/closure-checks.json.
Publicación:commit-result.json/push-result.json del mismo directorio local,
fuera del árbol versionado. No se copian logs extensos al commit.

## Límite y responsable siguiente

No acredita cadencia real,motor/MCU,configuración de periféricos o ejecución física.
La etapa de integración la implementa Claude por instrucción del usuario; Codex
no inicia ni propone otro bloque. La nota externa sobre posible revisión de
firma/retención en I2 queda para esa etapa; no modifica RV3 ni la API aceptada.

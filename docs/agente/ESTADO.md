# Estado — 2026-11-11

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  HEAD de incorporaciónb8c3c37, origin/main62eef53; consultar Git al reanudar.
  El commit de este documento entrega corrección de pruebas/README B05-R1.
- B00–B16/B03-R1/B07-R1 aceptados/archivados. B05-R1 activo, sin cierre,
  sincronización, archivo o push. Auditoría inicial Claude: aprobar con cambios;
  H1/H2 corregidos y verificados, reauditoría/aceptación pendientes.
  [Informe actual](../../openspec/changes/b05-r1-lectura-lm35/verify-report.md).
- Campos agrega0x00FF0000/0x38000000/0xFFFF000F; EmitirTabla usa0xFFFF000F y fixture
  ARM0xFFFF200F. S1 aclara todo bit fuera de15:4; README entrega centésimas directas.
- LM35.c y restante producción/configuración/servicios permanecen byte a byte
  respecto de b8c3c37. Drivers90 originales a21cc1e, contratos y archivo intactos.
  [Auditorías externas locales resumidas](../../openspec/changes/b05-r1-lectura-lm35/review-notes.md).

## Evidencia propia tras la corrección

Mutante externo (uint16_t)(ADC_DMA_LM35 >> 4): antes sobrevive4/4 casos y4096
códigos; después falla S1 y primer código del oráculo (300000 frente a0).
No se modifica el LM35.c versionado. Logs/copias/comandos fuera del repo:
`/tmp/ed3-b05r1-bits-11qd108k`.
Host/san29/29,214 ejecuciones C y20480 comparaciones exactas con0xFFFF000F;
4 probes positivos/4 negativos. ARM13/13 y Doxygen43 sin avisos. Sin pruebas
omitidas.16/16 tareas al commit de entrega; diff/hashes y enlaces revisados.
Recursos O0/O2/headers anteriores siguen informativos, no repetidos en este paso.
Sin ejecución MCU/placa, simulación, instalaciones, flasheo o publicación.

## Próxima acción y pendientes

Reauditoría de Claude sobre corrección entregada y aceptación del usuario.
Mantener cambio activo: no cerrar/sincronizar/archivar/push. Al integrar definir
origen del estado genérico de fuente/primera muestra; cero inicial no indica
disponibilidad. Resolver ADC/DMA, VREF/pin/canal físicos y riesgo ADGDR/ADDRn contra
UM10360/erratas/API original sin modificar Drivers/B16 ahora. Regulación automática,
servicios/main, placa y diagramas/informe siguen pendientes; inicialización al final.

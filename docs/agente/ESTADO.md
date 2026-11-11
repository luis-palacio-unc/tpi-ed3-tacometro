# Estado — 2026-11-11

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  HEAD de incorporación94d3134, origin/main62eef53; consultar Git al reanudar.
  El commit de este documento cierra B05-R1 localmente. Sin push autorizado.
- B00–B16/B03-R1/B07-R1/B05-R1 aceptados y archivados. B05-R1: usuario acepta
  94d3134; reauditoría de Claude r2 APROBAR, sin hallazgos abiertos.
  [Cierre y evidencia nueva](../../openspec/changes/archive/2026-11-11-b05-r1-lectura-lm35/archive-report.md).
- [Temperatura vigente](../../openspec/specs/temperatura/spec.md): TE1/TE3/TE4
  sincronizados; TE2/S4/S5 conservados byte a byte. LM35_Read devuelve int32_t
  centésimas desde muestra propia; sin estados de dispositivo. ADC_VREF_MV
  configurable, default2540mV provisional de software, no VREF física confirmada.
- Código, pruebas, build y90 Drivers originales conservados en este cierre.
  Historial archivado previo y revisiones externas intactos.
  [Resumen portable de auditorías](../../openspec/changes/archive/2026-11-11-b05-r1-lectura-lm35/review-notes.md).

## Evidencia propia de cierre

Host29/29 y ASan/LSan/UBSan29/29, sin pruebas omitidas:214 ejecuciones C,
20480 comparaciones racionales exactas y8 probes de configuración por modalidad.
Build ARM y13/13 controles de imagen/enlace; Doxygen HTML/XML43 funciones sin avisos.
Hashes, delta/contrato, artefactos y enlaces revisados; diff sin errores de formato.
Comandos/logs externos: `/tmp/ed3-b05r1-cierre-qe5tm7u1`. Evidencia anterior de mutación en verify-report.md
archivado; no repetida en el cierre. Sin ejecución MCU/placa, simulación o flasheo.

## Próxima acción y pendientes

Acordar el siguiente bloque; no hay uno nuevo autorizado. Inicialización e
integración quedan para el final. Resolver ADC/DMA real, VREF/pin/canal físicos,
origen del estado genérico de fuente/primera muestra (cero inicial no señala
disponibilidad), y riesgo de mezcla ADGDR/dos destinos: evaluar ADDRn por canal
contra UM10360/erratas/API original sin modificar Drivers. Regulación automática,
composición servicios/main, pruebas sobre placa y diagramas/informe pendientes.

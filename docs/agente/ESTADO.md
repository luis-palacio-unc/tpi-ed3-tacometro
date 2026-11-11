# Estado — 2026-11-11

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  HEAD de incorporación de la corrección24adfe3, origin/main62eef53. Consultar
  HEAD/estado al reanudar. El commit de este documento registra el traslado de
  ADC_DMA_Pot solicitado por el usuario. Commit local autorizado, sin push.
- B00–B15/B03-R1/B07-R1 aceptados/archivados. Contratos aceptados intactos.
- B16 implementado/verificado con corrección de propiedad; resultado pendiente
  de aceptación, sin sincronizar specs principales ni archivar.
  [Informe](../../openspec/changes/b16-lectura-potenciometro-dma/verify-report.md).
- ADC_DMA_Pot se define únicamente en Potenciometro.c; palabra volatile32bits,
  .bss4 bytes. Extern interno en Adquisicion_ADC_DMA.h para configurar DMA al final.
  Soporte no define datos; test del getter enlaza sin el .c de soporte.
- Pot_Read(void) conserva cuerpo/API: RESULT0..4095, sin parámetros/estado/validación.
  AD0.0/P0.23 función01 acordado; no se configura ni ejecuta ADC/DMA todavía.
- Drivers90 originales a21cc1e intocables; servicios/otros dispositivos preservados.
  Inicialización/enlace al final. B15 conserva MR0=1000 y envío directo0..1000.

## Evidencia actual tras el traslado

Host22/22 y ASan/LSan/UBSan22/22,207 casos C+oráculo; B16 tres casos de memoria.
ARM13/13 de imagen/enlace existentes; objetosO0/O2 confirman definición única en
Potenciometro y soporte sin símbolos/datos. Doxygen43 funciones/USW sin avisos.
Potenciometro.text32/16,marco propio4/0 y.bss4 alineada4; soporte0, no cota global.
Diff/hashes: biblioteca90 original, servicios/otros dispositivos/plataforma/specs
y archivos aceptados intactos.12/12 tareas al cierre técnico. Sin pruebas omitidas.
Logs: `/tmp/ed3-b16-propiedad-pot-wve2trjj`. Sin ejecución en MCU/placa,
simulación MCU, flasheo o publicación.

## Próxima acción y pendientes

Revisar/aceptar B16 antes de sincronizar/archivar o iniciar otro bloque.
Configurar ADC/DMA/pines/reloj, secuencia multicanal/ADC.2, asociación canal-destino,
VREF/tasa, integración con servicios/main y ensayos eléctricos al final.
SRAM local para MCU activo; revisar ubicación/relojes si se usa Sleep. Cero inicial
no indica disponibilidad; getter no garantiza edad/coherencia multicanal.
Sin push autorizado. No retomar B12 de enlace diferido. Diagramas del informe
al terminar los ESW; regulación e integración general pendientes.

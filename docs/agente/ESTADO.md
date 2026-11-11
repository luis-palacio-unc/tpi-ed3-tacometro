# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  HEAD de incorporación14b0521, origin/main62eef53. Consultar HEAD/estado al reanudar;
  el commit de este documento cierra técnicamente B16 sobre esos cuatro commits locales.
  Commit local autorizado; sin push.
- B00–B15/B03-R1/B07-R1 aceptados/archivados. Contratos aceptados intactos.
- B16 b16-lectura-potenciometro-dma: alcance acordado e implementado/verificado;
  resultado pendiente de aceptación, sin sincronizar specs principales ni archivar.
  [Informe](../../openspec/changes/b16-lectura-potenciometro-dma/verify-report.md).
- Pot_Read(void) devuelve0..4095 desde ADC_DMA_Pot con ADC_GDR_RESULT original;
  sin parámetros/estado/validación. Soporte posee palabra volatile32bits, .bss4 bytes.
- AD0.0/P0.23 función01 acordado. DMA será productor al integrar; hoy no se configura
  ni ejecuta ADC/DMA. Mandos/LM35/servicios/otros dispositivos preservados.
- Drivers90 originales a21cc1e intocables. Inicialización/enlace al final.
  B15 conserva MR0=1000 como precondición final y envío directo0..1000 a PWM_MATCH_1.

## Evidencia ejecutada B16

Host22/22 y ASan/LSan/UBSan22/22,207 casos C+oráculo. B16 tres casos: muestras
0/2048/4095, actualización/repetición y sólo RESULT sin escritura. ARM13/13
(controles existentes), objetosO0/O2 y enlace parcial sin externos; headersC11/C++17.
Doxygen43 funciones/USW sin avisos; negativos en copias externas detectados.
Getter.text32/16 bytes y marco propio4/0; soporte.bss4 alineado4, no cota global.
Diff/hashes: biblioteca90 original, servicios/otros dispositivos/plataforma/specs
y archivos aceptados intactos.9/9 tareas al cierre técnico. Sin pruebas omitidas.
Logs fuera del árbol: `/tmp/ed3-b16-getter-33p91_4l`.
Sin simulación MCU, ejecución en placa, flasheo o publicación.

## Próxima acción y pendientes

Revisar/aceptar resultado B16 antes de sincronizar/archivar o iniciar otro bloque.
Configuración ADC/DMA/pines/reloj, secuencia multicanal/ADC.2, asociación canal-destino,
VREF/tasa, integración con servicios/main y ensayos eléctricos para el final.
SRAM local elegida para MCU activo; si se agrega Sleep, revisar ubicación/relojes.
Cero inicial no informa disponibilidad; getter no garantiza edad/coherencia multicanal.
No push autorizado. No retomar B12 de enlace diferido. Diagramas del informe
al terminar todos los ESW; regulación e integración general siguen pendientes.

# Estado — 2026-11-11

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  HEAD de incorporación978af35, origin/main62eef53; consultar Git al reanudar.
  El commit de este documento entrega implementación B05-R1, sin cierre/archivo/push.
- B00–B16/B03-R1/B07-R1 aceptados/archivados. B05-R1 implementado/verificado,
  pendiente de auditoría de Claude y aceptación del usuario; specs principales intactas.
  [Informe](../../openspec/changes/b05-r1-lectura-lm35/verify-report.md).
- Claude r2 aprobó propuesta; usuario acordó implementación con m1–m3. Resumen
  [portable de revisiones externas locales](../../openspec/changes/b05-r1-lectura-lm35/review-notes.md).
  No depender de archivos del harness en clones públicos ni atribuir sus ensayos al código nuevo.
- LM35_Read devuelve int32_t centésimas desde muestra propia/extern interno;
  conversión entera con constantes nombradas, redondeo B05; sin estados/init/flotantes.
  ADC_VREF_MV configurable, default2540mV provisional y guard positivo/representable.
- Drivers90 originales a21cc1e y servicios/otros dispositivos preservados. Main
  sigue mínimo; no configuración/inicio ADC/DMA, IRQ o composición productiva nuevos.

## Evidencia propia B05-R1

Host/san29/29:214 ejecuciones C,20480 comparaciones exactas Fraction en4096 códigos
por5 referencias,4 probes positivos/4 negativos; fuentes y estados genéricos intactos.
ARM13/13 y objetosO0/O2, headersC11/C++17, Doxygen43 sin avisos. Test/default y
5 variantes sólo con LM35.c; defines privados correctos, firmware sin esos overrides.
Objeto LM35 .text268/32 bytes, .bss4 alineada4; marcosO0 Convert96+Read8, O2 Read0,
sin externos. No cota total de pila/tiempo real.13/13 tareas al commit de entrega.
Código/contratos/archivo ajenos/harness intactos; enlaces internos válidos.
Logs extensos fuera del repo: `/tmp/ed3-b05r1-impl-3lr7invk`.
Sin ejecución MCU/placa, simulación, instalaciones, flasheo o publicación.

## Próxima acción y pendientes

Auditoría de Claude sobre implementación entregada y revisión/aceptación del usuario.
Por orden actual, no cerrar, sincronizar contrato principal, archivar o hacer push.
Al integrar resolver ADGDR/dos destinos, evaluar ADDRn con UM10360/erratas/API original;
no asumir mitigación probada ni modificar Drivers/B16 ahora. VREF/pin/canal/primer dato,
ADC/DMA, regulación automática, enlace de servicios/main y placa siguen pendientes.
Diagramas/informe al terminar ESW; inicialización/enlace al final.

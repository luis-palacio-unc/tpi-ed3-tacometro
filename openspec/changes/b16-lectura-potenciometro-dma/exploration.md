# Exploración B16 — 2026-11-10

Base b473d0f, main. B15 aceptado; queda Potenciometro (ESW.2.1.12), soporte
ADC/DMA y regulación/integración. Mandos B04 normaliza lectura/fondo inclusivo;
LM35 B05 usa muestra/escala/referencia propios. No duplicar esas conversiones.

LPC17xx original: ADC_ChannelGetData/ADC_GlobalGetData leen registros, no bloques
DMA. lpc17xx_adc.h ya define macros ADC_GDR_RESULT/CH y flags. GPDMA_LUTPerAddr
asocia ADC a ADGDR y SetupChannel selecciona esa tabla para P2M, incluso si se
suministra srcMemAddr. Esta lectura/validación de un dato en RAM necesita usar
macros, no volver a leer el periférico ni alterar la biblioteca.

UM10360 rev4.1 tabla81/p118: AD0.0/P0.23 función01, coincide con ADC_PinConfig
canal0. §29.5.2/tabla533/p589: resultado15:4, canal26:24, OVERRUN30 y DONE31.
No mezclar lectura de registros global/individual porque sus flags pueden
desincronizarse. Páginas completas renderizadas e inspeccionadas en /tmp/ed3-cierre-b15-propuesta-b16-zasor5qr/manual-*.png;
logs Poppler externos. Cableado/conector/revisión/VREF reales aún por verificar.

[NXP ES_LPC176x rev10.6 §3.2 ADC.2/p7](https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf)
limita ADGDR con burst/disparo hardware; conversiones por software exigen consumir
el resultado anterior antes de iniciar otra. La configuración final deberá
resolver esa secuencia, fuente/anchura/burst DMA y propiedad de buffers, usando
drivers originales. Este bloque sólo decodifica palabras ya transferidas y no
elige un modo incompatible ni afirma haber resuelto adquisición física.

Propuesta: ADC_Decode y Pot_Read sin estado/contexto nuevos. Entrada por valor
debe venir de buffer completo/estable; campos válidos no prueban frescura o
exactitud eléctrica. Prioridades definidas y salida cero ante fallo, con estado
explícito. Recurso/pin/API propuestos, sin acuerdo/código/pruebas B16 ejecutados.

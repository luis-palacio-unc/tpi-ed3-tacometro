# B05-R1 — LM35: muestra y lectura en centésimas Celsius

Propuesta corregida2026-11-11 tras revisión de Claude y decisiones del usuario.
Propuesta aprobada por Claude r2 y ejecución acordada por usuario; implementación verificada, pendiente de auditoría del código y aceptación del resultado.

## Objetivo y origen

R07/R08/R09/R10/R11/R15: muestra propia, conversión y wrapper sin estados LM35.
Fuente: [resumen de revisiones externas locales](review-notes.md), incluidas
las decisiones finales del usuario y aprobación r2. B05 vigente continúa hasta aceptar el resultado revisado.

## Alcance

- ADC_DMA_LM35 en LM35.c y extern interno para futuro DMA.
- int32_t LM35_Read(void), con conversión entera interna LM35_Convert(muestra).
  Devuelve centésimas Celsius:2500 representa25,00°C. Sin float/double ni estados.
- Conservar redondeo B05: cercano, empate arriba, error aritmético≤0,005°C.
  Extraer sólo RESULT mediante macro original; sin recorte ni redondeos intermedios.
- ADC_VREF_MV configurable por #ifndef con default provisional2540mV y #error
  para valores no positivos/no representables. Default de software, no VREF medida.
- Retirar enum/registro/inicializador/API anterior; documentar IDs nuevos. Conservar
  unidades y TE2 del servicio; el fixture entrega centésimas directamente.

## Exclusiones y dependencias

ADC/DMA/pin/canal/reloj/IRQ/primera adquisición/main para el final. Sin filtros,
diagnóstico de desconexión, regulación o alarma nuevos. Drivers intocables.
Anteproyecto anexoA/p4 rotula +2V54DC_VREF; motiva default provisional, no confirma
la placa. Riesgo ADGDR/dos destinos y evaluación de ADDRn se registran en diseño;
origen/secuencia/erratas se verifican al integrar, sin modificar B16 ahora.

Archivos afectados al implementar: LM35.h/.c, soporte/Adquisicion_ADC_DMA.h,
Configuracion/Analogica.h, test_lm35.c, test_integracion_temperatura.c,
CMakeLists.txt, cmake/verificar_enlace_b05_lm35.cmake y documentación/SDD.

## Aceptación

1. VREF de prueba2000mV: muestras0/512/2048→0/2500/10000 centésimas, también
   con DONE/OVERRUN/CHN/bits3:0 activos; memoria intacta y actualización sin caché.
2. S2 define targets con overrides de compilador independientes del firmware y
   builds negativos0/−1 rechazados por el diagnóstico previsto.
3. S3 compara exactamente4096 códigos por referencia1000/2000/2540/3300/5000mV
   contra oráculo racional independiente, incluyendo empate hacia arriba.
4. Compatibilidad directa con Temperatura, host/san, ARM y Doxygen pasan;
   biblioteca/otros módulos/contratos aceptados intactos, evidencia física pendiente.

Riesgos: mezcla de canales y referencia errónea; resolver al integrar. Medir recursos
de aritmética entera. Reversión por commit inverso acotado. Commit al terminar bloque
acordado; sin cierre, sincronización de contratos principales, archivo ni push.

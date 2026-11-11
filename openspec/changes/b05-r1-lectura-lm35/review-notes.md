# Revisiones de propuesta B05-R1 — resumen portable

Claude revisó la propuesta inicial y su corrección el 2026-11-11. Documentos de
origen: `B05-R1-propuesta.md` y `B05-R1-propuesta-r2.md`, **revisiones externas
locales**, no distribuidas con el repositorio ni necesarias para compilarlo.
Este resumen conserva las decisiones y observaciones aplicables al cambio.
No se publican el harness, sus scripts ni sus logs privados.

## Decisiones incorporadas

| Hallazgo | Decisión |
|---|---|
| H1 | int32_t en centésimas, sin float; conservar redondeo B05 cercano/empate arriba y error aritmético≤0,005°C; oráculo racional exacto4096 códigos |
| H2/H7 | ADC_VREF_MV con #ifndef/default2540 provisional/#error; overrides privados por target, builds negativos; default no atribuido a placa |
| H3 | Riesgo ADGDR/dos destinos registrado; evaluar ADDRn/UM10360/erratas/API original al integrar; no modificar Drivers o B16 ahora |
| H4 | S1/S6 con DONE/OVERRUN/CHN/bits3:0 activos, además de palabras limpias |
| H6 | Retirar USW11.1/11.2; getter11.3 y conversión interna nueva11.4, sin reciclar |
| H5 | No aplicable al haberse retirado la propuesta float |

## Segunda revisión y autorización

Veredicto r2: **APROBAR** la propuesta corregida. Claude informó comprobaciones
de los ejemplos aritméticos, las cotas de tipos, el guard de configuración y el
comportamiento de la selección de origen GPDMA. Esos resultados son evidencia
externa de planificación, no pruebas de una implementación todavía inexistente
en el momento de esa revisión. La verificación propia se registra en verify-report.md.

El usuario acordó implementar el alcance con m1–m3 y crear commit de implementación,
sin cierre, sincronización de contratos principales, archivo o push.

| Observación r2 | Incorporación |
|---|---|
| m1 | Constantes con nombre para bits/cuentas ADC, mV/°C y centésimas/mV; factor derivado de100/10 con explicación Doxygen |
| m2 | Targets de test con LM35.c como único módulo productivo; muestra definida una sola vez y sin Adquisicion_ADC_DMA.c |
| m3 | Referencias externas identificadas como locales, sin enlaces a rutas fuera del clon; decisiones resumidas aquí |

La aprobación r2 corresponde a la propuesta. El resultado implementado queda
pendiente de auditoría posterior y aceptación del usuario.

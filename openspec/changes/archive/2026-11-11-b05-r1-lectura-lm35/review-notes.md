# Revisiones B05-R1 — resumen portable

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

Esa aprobación r2 corresponde a la propuesta; en aquella entrega quedaban
pendientes la auditoría del código y la aceptación, resueltas al final de este resumen.

## Auditoría de implementación sobre b8c3c37

Fuente: `B05-R1-implementacion.md`, revisión externa local no distribuida; resumen
portable, sin enlace a archivos fuera del clon. Veredicto de Claude: aprobar con
cambios, sin bloqueantes. Código LM35 correcto; hueco de pruebas en bits reservados.

H1: activar23:16/29:27 en campos y usar0xFFFF000F en EmitirTabla. Comprobar que
cast tras desplazar sin máscara falle en copia externa, sin cambiar LM35.c.
H2: README debe describir centésimas entregadas directamente, sin reescalar.
H3 informativo: origen del estado genérico de fuente y primera muestra se acuerdan
en la integración final; cero inicial no señala disponibilidad. Sin cambios ahora.

La reproducción propia y los resultados de la corrección se registran en
verify-report.md, separados de la evidencia externa de Claude. Aquella corrección
se entregó sin cierre, sincronización, archivo o push, según su autorización.

## Reauditoría de implementación r2 y aceptación

Fuente: `B05-R1-implementacion-r2.md`, revisión externa local no distribuida,
2026-11-11 sobre94d3134. Veredicto de Claude: **APROBAR**, sin hallazgos abiertos.
H1/H2 corregidos; se conserva H3 como responsabilidad de integración final
(origen del estado genérico y primera muestra, sin añadir estados al dispositivo).
Claude informó verificaciones propias de host/san/ARM/Doxygen y de sensibilidad
de las pruebas. Son evidencia externa, separada de las nuevas ejecuciones de Codex.

El usuario aceptó94d3134 y autorizó sincronizar TE1/TE3/TE4 conservando TE2/S4/S5,
archivar y crear commit local de cierre, sin push. La verificación propia de cierre,
sus comandos y límites constan en [archive-report.md](archive-report.md).

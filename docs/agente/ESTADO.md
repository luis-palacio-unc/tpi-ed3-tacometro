# Estado — 2026-11-10

## Punto actual

- Repositorio `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`; rama main, consultar HEAD al incorporarse.
- B00–B10/B03-R1 aceptados/archivados; base local B11 TC db88286 (SysTick no aceptado,
  sin push), base publicada b4981c9. Historia preservada, sin reescritura.
- B11 TC/Tiempo: alcance acordado, implementado/verificado; resultado pendiente
  de aceptación. Activo b11-base-temporal-timer-tc, candidato TC1–TC5/S1–S8.
  Contratos vigentes/archivos aceptados intactos; planificación local previa incorporada.
- Commit normal local autorizado: `feat: reemplazar SysTick por TIMER0 y wrapper temporal`;
  consultar Git para hash. Sin push autorizado. Antecedente SysTick superseded.

## Evidencia ejecutada

| Modalidad | Resultado |
|---|---|
| Host/san | 20/20, 209 C; 16 TC +9 wrapper +5 integración, oráculo2048 muestras/1235 vueltasTC, regresiones11199/5096/4096 |
| Refuerzo posterior | Sólo TC host/san1/1 por nuevas aserciones de fronteras, código productivo intacto |
| ARM | Debug15/15; fixture API completa Debug/Release PASS, sin ejecutar MCU |
| Doxygen | 42 funciones/USW, diez TC/Tiempo, sin avisos |
| Negativos | Detecta PR fuera de uno, consulta que reinicia TC, ISR propia y default cero; overrides2/5ms PASS |
| Recursos | .text926 con helpers CMSIS (sin libgcc), .data0/.bss5 antes de alineación; ruta proveedor conocido108 bytes Debug antes de llamador/IRQ |
| Preservación | Servicios previos/main/Drivers/startup/linker/tests C/specs/archivos aceptados intactos; B00 S7 adaptado a19 objetos, conserva14 casos |

Informe activo verify-report.md con matriz, recursos, límites y comandos. Logs externos:
/tmp/ed3-b11-tc-20261110-j1_ngpam. Incidencias resueltas: expectativa B00 de18 objetos y fixture Release sin llamada
directa a API eliminada por inline/GC; controles no relajados. Sin fallos pendientes.
No ejecución ARM/simulación MCU/placa, flasheo/instalación/cambios sensibles MCU.

## Próxima acción y límites

Revisar/aceptar resultado B11 antes de sincronizar/archivar/publicar por orden.
No avanzar bloque posterior. TIMER0 único en dispositivos, TC continuo, sin ISR/
acumulador; PCLK explícito y período1ms por defecto configurable, marcas siempre ms.
Tiempo wrapper genérico con proveedor; consultas/intervalos desde marcas propias,
misma época y duración real<2^31ms. Reiniciar fuente exige nuevas marcas.
APIs dispositivo sólo lazo privilegiado, timer/PCONP/configuración serializados.
main no inicia timer, PCLK real/precisión/suspensión/debug sin verificar; 25MHz de
fixture no es dato confirmado de placa. Adquisición/ventanaRPM1s, GPIO/selector,
UART/TX, PWM/regulación y main pendientes. Tiempo ms distinto de orden B01.
CMSIS-DAP integrado; revisión PCB/silicio/pines/VREF, umbral térmico TPI y política
física/reentrada pendientes. Diagramas finales diferidos.

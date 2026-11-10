# Estado — 2026-11-10

## Punto actual

- Repositorio `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`;
  rama main, consultar HEAD. Corrección de B11 sobre 86107b9, sin reescribir historia.
- B00–B10/B03-R1 aceptados/archivados. B11 candidato revisión3 implementado/verificado;
  resultado pendiente de aceptación, no sincronizado/archivado. Base publicada b4981c9.
- Timer reemplaza Base_Tiempos/puerto/adapter; API Init/Read/Stop/Source. Usa driver
  TIM existente y extensión compatible TIM_InitFreeRunning que conserva PCLK.
  Tiempo wrapper independiente conserva API/implementación. Main aún mínimo.
- Commit normal local autorizado al cierre: `refactor: simplificar Timer y reutilizar drivers LPC17xx`.
  Consultar Git para hash; sin push autorizado.

## Evidencia actual ejecutada

| Modalidad | Resultado |
|---|---|
| Host y ASan/LSan/UBSan | 21/21 cada uno, 212 C: 16 TC +9 wrapper +5 integración +3 driver y179 previos |
| Referencias | Tiempo2048 muestras/1235 vueltasTC; regresiones velocidad11199, codec5096, selector4096/400 vueltas |
| ARM | Debug15/15 y Release15/15, fixture con APIs/drivers reales, vectores por defecto; sólo compilación/inspección |
| Doxygen | 38 funciones propias únicas, seis TC/Tiempo, sin avisos; contrato nuevo driver revisado en header |
| Negativos | Detecta PR fuera de uno, lectura que reinicia TC y cambio PCLK; bases1/2/5 PASS, cero rechazado |
| Recursos fixture | Flash3684 Debug/2988 Release, SRAM2108 incluyendo pila2048; .bss Timer5, Tiempo0 antes de alineación |
| Preservación | Aplicación/main/startup/linker/servicios/contratos/archivos aceptados intactos; driver extendido y licencias/aserciones conservadas |

Informe y comandos en `openspec/changes/b11-base-temporal-timer-tc/verify-report.md`.
Logs externos: /tmp/ed3-b11-drivers-20261110-qhso3a72. Evidencia anterior corresponde a 86107b9 en Git;
SysTick db88286 conserva su cambio superseded. Incidencias de compilación de biblioteca
resueltas sin desactivar controles. No ejecución MCU/simulación/placa ni flasheo/instalación.

## Próxima acción y límites

Revisar/aceptar B11 corregido antes de sincronizar/archivar/publicar por orden.
No iniciar otro bloque. Timer0 exclusivo, APIs/PCONP serializados desde lazo privilegiado;
PCLK real estable explícito, default1ms configurable, salida siempre ms, sin ISR/acumulador.
Marcas misma época y duración real<2^31ms; reinicio exige marcas nuevas.
Faltan PCLK/precisión/suspensión/debug físicos, revisión PCB/silicio/pines/VREF,
adquisición/ventanaRPM1s, GPIO/selector, UART/TX, PWM/regulación y main integrado.
CMSIS-DAP integrado confirmado; umbral térmico TPI/reentrada pendientes. Diagramas finales diferidos.

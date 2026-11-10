# Estado — 2026-11-10

## Punto actual

- Repositorio: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD con Git.
- B00–B10 y B03-R1 aceptados/archivados; base publicada B11:
  b4981c92a300243c8b7d960d09a99aa81c1ce300, con cambio remoto README integrado.
- B11: alcance/criterio acordados el 2026-11-09; implementado/verificado,
  resultado pendiente de aceptación. Cambio activo b11-base-tiempos-lpc1769;
  contrato candidato BT1–BT4/S1–S8, contratos vigentes/archivos aceptados intactos.
- Commit local autorizado con asunto `feat: agregar base temporal SysTick con reloj explicito`;
  consultar Git para hash. Sin push autorizado; conservar hooks/firma/historial.

## Evidencia ejecutada

| Verificación | Resultado |
|---|---|
| Host/san | 20/20; 205 C (18 base + cuatro integración + cuatro puerto), oráculos selector 4096/400, codec 5096 y velocidad 11199 |
| ARM | Debug 15/15 y fixture B11 Release PASS; vector fuerte/API retenida, sin ejecución |
| Doxygen | 40 funciones/USW; ocho nuevas .2.1.17.1–.8, sin avisos |
| Negativos | Detecta pendiente antiguo, recarga fuera de uno y vector débil |
| Recursos -O0 | .text 636 con helper CMSIS; .data 0/.bss 5 antes de alineación; cadena propia 64 antes de llamador/IRQ/otros |
| Preservación | Servicios/aplicación/main/Drivers/startup/linker, pruebas anteriores, contratos vigentes e historial aceptado intactos |

Matriz y recursos en verify-report.md activo. Logs externos: /tmp/ed3-b11-20261109-br0hi5vv.
Se corrigió documentación void de Doxygen; primer intento LSan falló por ptrace,
repetido fuera de seguimiento con fugas habilitadas: PASS. Sin fallos pendientes.
No hubo ejecución ARM/simulación MCU/placa, flasheo/instalación/cambios sensibles MCU.

## Próxima acción y límites

Revisar resultado B11; sólo después sincronizar/archivar por orden. No publicar ni
iniciar otro bloque sin autorización. Base única SysTick, frecuencia CPU explícita;
APIs sólo lazo privilegiado serializado. Época nueva exige reiniciar consumidores.
Sólo ACTIVA habilita consumo; pendientes antiguos cancelados, ticks colapsados no
recuperables. main no inicia SysTick ni configura reloj. Frecuencia/latencia/precisión
físicas, GPIO/selector, ADC/DMA/captura/ventana RPM 1 s, UART/TX, PWM/regulación y
composición real pendientes. Tiempo ms no equivale a orden B01.
CMSIS-DAP integrado; revisión PCB/silicio/pines/VREF sin verificar. Umbral térmico
TPI y política física/reentrada tras pérdida pendientes. Diagramas finales diferidos.

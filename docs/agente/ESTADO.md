# Estado — 2026-11-09

## Punto actual

- Repositorio: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD con Git.
- B00–B09 y B03-R1 aceptados/archivados. Base publicada B10: 9f53e73e0841123cfba29ec3dbbbb2bab567cf03.
- B10: alcance/criterio acordados, implementado/verificado; resultado pendiente de aceptación.
  Cambio activo b10-seleccion-estable-modo; contrato vigente/archivos aceptados intactos.
- Commit local autorizado con asunto `feat: filtrar seleccion de modo con tiempo externo`;
  consultar Git para hash. Sin push autorizado; conservar hooks/firma/historial.

## Evidencia ejecutada

| Verificación | Resultado |
|---|---|
| Host/san | 17/17; 179 C (14 selector + cuatro integración), referencia 4096 muestras/400 rollovers; codec 5096 y velocidad 11199 |
| Refuerzo temporal | Sólo prueba ampliada, sin cambios productivos: selector host/san 1/1; sin hallazgos |
| ARM | Debug 13/13 y fixture B10 Release PASS; API retenida/ELF válido, sin ejecución |
| Doxygen | 32 funciones/USW, nuevas USW.2.1.5.3/.4/.5; sin avisos |
| Negativos | Detecta falta de reinicio de candidato y frontera tardía |
| Recursos -O0 | Selector .text 398, .data/.bss 0; pila propia anidada 40 excluye llamador/IRQ/otros; contexto 16 y enum 1 con ABI actual |
| Preservación | B04 cuerpos/tipos/constantes/USW intactos; B01/B08/main/otros servicios/dispositivos/Drivers, pruebas anteriores y contratos/archivos aceptados intactos |

Matriz MA3/S6 y SE1–SE4/B10-S1–S6 en verify-report.md activo. Logs externos: /tmp/ed3-b10-20261109-1jdibt8h.
Sanitizadores sin ptrace, fugas habilitadas. Sin fallos pendientes; sin ejecución
ARM, simulador o placa, flasheo/instalación/cambios sensibles MCU.

## Próxima acción y límites

Revisar resultado B10; sólo después sincronizar/archivar por orden. No publicar ni
iniciar otro bloque sin autorización. Selector lógico exige contexto inicializado,
tiempo/muestras coherentes y acceso serializado; GPIO/timer/cadencia físicos y main
pendientes. B01 usa orden distinto de ms; adaptación sólo en pruebas/fixture.
Política física/reentrada tras pérdida, ADC/DMA/captura, UART/TX, PWM/regulación pendientes.
CMSIS-DAP integrado; revisión PCB/silicio/pines/VREF sin verificar.
Diagramas finales después de completar ESW, según CONTEXTO.

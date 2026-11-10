> POSPUESTO por el usuario el 2026-11-10: inicialización e integración de enlace
> quedan para el final. Esta propuesta no fue aceptada ni implementada.
> La propuesta activa B12 es b12-dispositivos-gpio.

# B12 — enlace y arranque de Timer

## Objetivo y origen

R01/R10/R15, B00/B11 y pendientes acordados: que el firmware arranque el timer
mediante los wrappers aceptados y pueda enlazar consultas con la biblioteca original.
Propuesta pendiente de aceptación; no autoriza implementación.

## Alcance

- Incorporar al enlace ARM las fuentes originales necesarias de TIM/CLKPWR/CMSIS
  y soporte de aserciones; resolver compatibilidad del build fuera de Drivers.
- Tras .data/.bss, actualizar SystemCoreClock mediante SystemCoreClockUpdate
  existente. Es lectura de configuración y escritura de la variable RAM, sin
  invocar SystemInit ni cambiar PLL/flash/protecciones.
- En main, llamar Timer_Init y Timer_Start una vez antes del lazo. Mantener API
  B11 y base1ms configurable; probar consulta con un fixture ARM de integración.
- Conservar los90 archivos de Drivers originales y la separación de capas.

Archivos previstos: CMakeLists.txt, plataforma/startup_lpc1769.S, Aplicacion/main.c,
soporte de compilación bajo cmake/plataforma si resulta necesario, controles de
build/enlace y documentación. Dispositivos/Timer.h/.c conserva contrato.
Excluir GPIO/selector físico, nuevos servicios, ventanaRPM, ADC/UART/PWM/control,
cambios de reloj activos, instalaciones y flasheo. Conexión de consumidores posterior.

## Dependencias y decisiones

B11 aceptado, toolchain existente. Biblioteca usa SystemCoreClock y algunos includes
en minúsculas; hay que resolver integración en Linux sin editar/copiar-parchear
sus fuentes. Fuente/reloj físicos pendientes; no asumir frecuencia real.

## Criterio de aceptación

1. ELF Debug/Release y fixture de lectura enlazan drivers reales, sin indefinidos.
2. Verificación de orden: .data/.bss→actualizar información de reloj→main→Init→Start.
3. Actualizar reloj no reconfigura PLL/flash; Timer_Read conserva ms y no reinicia TC.
4. Regresiones/controles de imagen y Doxygen pasan; hashes Drivers intactos,
   sin relajar controles para obtener verde ni afirmar pruebas físicas no ejecutadas.
5. Ensayo físico de avance/base1ms queda identificado para cuando se autorice
   programación y haya placa; compilación no acredita tiempos reales.

Reversión: commit inverso aislado de B12, conservando B11/biblioteca/historia.

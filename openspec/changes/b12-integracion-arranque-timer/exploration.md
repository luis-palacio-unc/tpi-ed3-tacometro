# Exploración B12 — sólo lectura

En c543fee Timer compila como objeto; main tiene un lazo vacío y el enlace ARM
no incluye fuentes TIM/CLKPWR. GC descarta las funciones no usadas. SystemCoreClock
es dependencia del cálculo TIM_US; no basta su valor inicial de compilación.

CMSIS original ofrece SystemCoreClockUpdate: consulta PLL0STAT/CLKSRCSEL/CCLKCFG y
actualiza RAM, sin escribir registros de configuración. SystemInit sí configura
PLL/flash y queda fuera del alcance. No inventar frecuencia física a partir de macros.
Comprobación de sintaxis ARM-C11 del system_LPC17xx.c original ejecutada PASS;
no prueba enlace/arranque/reloj. Log externo en el directorio de cierre B11.

Drivers originales incluyen lpc17xx.h en minúsculas desde CLKPWR/PINSEL, mientras
el encabezado CMSIS se llama LPC17xx.h. Resolver sólo desde integración externa,
sin modificar biblioteca ni producir copias corregidas. Revisar además diagnósticos
de compilación de fuentes originales y conservar aserciones/controles.
No implementar B12 hasta aceptar su alcance.

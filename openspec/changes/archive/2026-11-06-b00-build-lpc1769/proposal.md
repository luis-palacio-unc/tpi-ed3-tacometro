# B00 — Compilación ARM mínima y verificable

Estado: alcance, implementación y resultado aceptados el 2026-11-06.

## Objetivo y origen

Convertir el esqueleto en una imagen ARM reproducible para LPC1769, con evidencia
estática verificable. Origen: R01 (plataforma del anteproyecto) y R13 (validación SDD).

## Alcance y enfoque

- Configurar toolchain ARM existente, Cortex-M3/Thumb y C11 en CMake.
- Incorporar startup, tabla de vectores, linker script y main mínimo sin periféricos.
- Usar encabezados CMSIS existentes; revisar su compatibilidad. No incorporar
  automáticamente SystemInit con su configuración heredada de PLL/flash.
- Generar ELF, mapa y reporte de memoria; documentar comandos desde build limpio.
- Agregar verificaciones estáticas automatizadas de ELF, vectores y regiones.

Excluye control, GPIO, PWM, ADC/DMA, UART, cambios de reloj/PLL, CI, simulación,
instalaciones y flasheo. No acredita ejecución física del arranque.

## Dependencias y decisiones

GCC ARM/binutils ya disponibles. Acordar rama inicial (`main` propuesta), consultar
manual NXP accesible y definir mapa de memoria/startup antes de escribirlos.
Revisión física pendiente; no se seleccionarán ajustes dependientes de revisión.
Commits y push quedan fuera mientras no se autoricen.

## Archivos afectados

`CMakeLists.txt`, `Aplicacion/main.c`, futuros `cmake/`, `plataforma/` y
`scripts/` de verificación. Actualizar documentación SDD y estado.

## Entradas, salidas y errores

Entrada: fuentes, configuración CMake y toolchain declarado.
Salidas: ELF ARM, mapa, uso de memoria y resultados de verificaciones.
Toolchain ausente, símbolo requerido faltante o desborde de memoria: fallo explícito;
no reemplazar silenciosamente ARM por compilador host.

## Escenarios y aceptación

- Build limpio → ELF ARM Cortex-M3/Thumb enlazado con main y Reset_Handler.
- Inspección → vectores ubicados, pila alineada dentro de SRAM, reset en flash y
  secciones sin superposición, dentro de los bancos definidos por fabricante.
- Revisión del startup → inicialización de .data/.bss y transferencia a main coherentes.
- Configuración sin toolchain o fixture que desborda memoria → error verificable.
- Build host previo sigue disponible como comprobación de objetos, sin presentarlo
  como firmware ejecutable.

Pruebas: build cruzado e inspección estática automatizada, con casos negativos.
Evidencia: comandos/versiones, ELF/mapa, reporte de memoria y resultados. Unitarias
funcionales host, simulación y hardware: no aplican al alcance; no ejecutadas hoy.

## Riesgo y reversión

Riesgo: confundir enlace correcto con arranque físico. Registrar esa limitación.
Reversión: retirar sólo los cambios propios de B00 y restaurar el CMake previo;
no usar reset destructivo ni alterar Drivers o cambios ajenos.

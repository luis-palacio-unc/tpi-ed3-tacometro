# Estado — 2026-11-11

## Punto actual

- Repo /home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII,main.
  HEAD aceptado8f1793f; el commit de este documento cierra B17. Consultar Git al reanudar.
- B00–B17/B03-R1/B07-R1/B05-R1 aceptados y archivados. B17: reauditoría de Claude r2
  APROBAR,sin hallazgos nuevos; aceptación explícita del usuario sobre8f1793f.
  [Contrato vigente](../../openspec/specs/regulacion-velocidad/spec.md) y
  [cierre](../../openspec/changes/archive/2026-11-11-b17-regulador-velocidad/archive-report.md).
- RV1–RV6/S1–S10 sincronizados íntegros;21/21 tareas. Cambio activo retirado.
  El cierre sólo cambia documentación/metadatos. Producción,pruebas,build,
  Drivers y contratos/archivo previos conservados.

## Integración en curso (B18)

- I1 (arranque y periféricos) **implementado y verificado en host/ARM, pendiente de
  revisión de Claude y de commit**; rama `integracion-i1`, base 6072762.
  Cambio `openspec/changes/b18-integracion/` (vía rápida aprobada por el usuario).
- Host/san 36/36 y host 36/36; ARM Debug 17/17 y Release 17/17; Doxygen 62 sin
  avisos; `Drivers/` sin cambios. Nada ejecutado en placa.
- El reloj lo configura `Reloj_Init` (FLASHCFG a 5 ciclos, `SystemInit`,
  `SystemCoreClockUpdate`); el startup no cambió. I2 debe llamarla primero en main.
  [Informe](../../openspec/changes/b18-integracion/verify-report.md).
- Specs vigentes sin sincronizar; `Aplicacion/`, `Servicios/` y `main.c` intactos.
  Siguen I2 (servicios simples, Aplicación y main) e I3 (MCUXpresso).

## Evidencia aceptada

Ejecución propia sobre8f1793f:host/san33/33 cada uno,10 casos PI+2 composición,
226 casos C por modalidad,8 configuraciones/133758 pasos exactos y25 probes.
Cinco mutantes detectados; copia correcta pasa. Planta14/14 pasos,ARM15/15,
Doxygen46 sin avisos,instancia40bytes. No se repiten suites en el cierre documental.
[Informe archivado](../../openspec/changes/archive/2026-11-11-b17-regulador-velocidad/verify-report.md).
Evidencia ignorada en workspace:build/verificacion-b17-h1-h2-2026-11-11;
controles/preservación/publicación del cierre:build/cierre-b17-2026-11-11.
Sin ejecución MCU/placa/motor ni aceptación física; sensor desconectado fuera de alcance.

## Continuidad y Git

Usuario autoriza commit local y push de main con los commits pendientes/cierre.
Comprobar rama/HEAD/origin con Git; datos de publicación en la evidencia del cierre.
La integración la implementa Claude. Codex termina este cierre y se detiene:
no iniciar ni proponer otro bloque. Quedan cadencia real,main/ISR/Timer/PWM,
ADC/DMA/VREF/primera muestra/asociación de canales/erratas,sintonía y ensayos físicos.
Nota externa de reauditoría: posible revisión de firma/retención al integrar I2,
sin alterar aquí el contrato aceptado.

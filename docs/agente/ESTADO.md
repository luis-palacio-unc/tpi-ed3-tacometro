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

- I1 (arranque y periféricos) revisado por Claude y en `main` (ff07e3e).
- I2 (servicios simples, Aplicación y `main`) revisado por Claude y en `main`
  (e3a279c). Cambio `openspec/changes/b18-integracion/` (vía rápida).
- Capas: Aplicación → Servicios → Dispositivos, comprobado por `i2.capas`. Cada
  servicio llama a sus dispositivos; sin contextos, callbacks ni enums de error.
  `Sistema_Init` arranca reloj y ADC/DMA. Modos de 3 estados; alarma 45/43 °C con
  promedio de 4 lecturas; PI de B17 sin cambios. Lazo: alarma → modo → mando →
  telemetría cada 500 ms; en MANUAL el potenciómetro actúa en cada vuelta.
- Host/san 29/29 y host 29/29; ARM Debug 9/9 y Release 9/9; Doxygen 60 funciones
  sin avisos; `Drivers/` y `Dispositivos/` sin cambios. Firmware Debug 19 636 B de
  flash y 2 456 B de SRAM (Release 13 124 / 2 448). 45 de 48 mutantes detectados,
  3 equivalentes. Nada ejecutado en placa.
  [Informe](../../openspec/changes/b18-integracion/verify-report.md).
- Contratos de mandos, medicion-velocidad, temperatura (servicio), mando-motor,
  salidas-alarma, comunicacion, modos y alarma-temperatura reemplazados por delta
  specs en el cambio, más `sistema`; specs vigentes sin sincronizar hasta cerrar.
- I3 (proyecto MCUXpresso) implementado, compilado sin interfaz y revisado por
  Claude (compilación headless reproducida); rama `integracion-i3`, base e3a279c (I1+I2). La
  raíz es el proyecto `TPI_ED3_Tacometro` (`.project`/`.cproject`, LPC1769), con las
  mismas fuentes que CMake, arranque del asistente en `plataforma/mcuxpresso/`,
  NewlibNano sin E/S y script de enlace administrado. MCUXpresso IDE 25.6.136:
  Debug y Release 0 errores y 0 avisos; `.axf` Debug 19 032 B de flash y Release
  10 652 B, 396 B de SRAM sin pila; vectores 17/21/42 propios, `ResetISR` sin
  `SystemInit`, CRP sin protección. CMake host/san 29/29, ARM 9/9 y 9/9, Doxygen 60
  funciones sin avisos. Importación por la interfaz, sonda y depuración sin probar.
  Guía y lista de puesta en marcha: [MCUXPRESSO.md](../MCUXPRESSO.md).
  Sigue I4 (puesta en marcha en placa, a cargo del usuario).

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

# Estado — 2026-11-19

Firmware aprobado y hardware terminado: ver «Cierre del firmware (2026-11-19)» al
final. Lo que sigue hasta esa sección es historial.

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

## Integración B18: aceptada y archivada (2026-11-11)

- Cierre ordenado por el usuario sobre e3d3bce: 17 deltas sincronizados en
  `openspec/specs/` (incluido el dominio nuevo `sistema`) y cambio movido a
  `openspec/changes/archive/2026-11-11-b18-integracion/`
  ([cierre](../../openspec/changes/archive/2026-11-11-b18-integracion/archive-report.md)).
  Sin aceptación física: la puesta en marcha en placa (I4) sigue pendiente y sus
  ajustes irán en un cambio nuevo.

- I1 (arranque y periféricos) revisado por Claude y en `main` (ff07e3e).
- I2 (servicios simples, Aplicación y `main`) revisado por Claude y en `main`
  (e3a279c). Cambio de vía rápida, hoy archivado.
- Capas: Aplicación → Servicios → Dispositivos, comprobado por `i2.capas`. Cada
  servicio llama a sus dispositivos; sin contextos, callbacks ni enums de error.
  `Sistema_Init` arranca reloj y ADC/DMA. Modos de 3 estados; alarma 45/43 °C con
  promedio de 4 lecturas; PI simplificado en I5 (ver abajo). Lazo: alarma → modo → mando →
  telemetría cada 500 ms (1 s desde el 2026-11-16, ver al final); en MANUAL el
  potenciómetro actúa en cada vuelta.
- Host/san 29/29 y host 29/29; ARM Debug 9/9 y Release 9/9; Doxygen 60 funciones
  sin avisos; `Drivers/` y `Dispositivos/` sin cambios. Firmware Debug 19 636 B de
  flash y 2 456 B de SRAM (Release 13 124 / 2 448). 45 de 48 mutantes detectados,
  3 equivalentes. Nada ejecutado en placa.
  [Informe](../../openspec/changes/archive/2026-11-11-b18-integracion/verify-report.md).
- Contratos de mandos, medicion-velocidad, temperatura (servicio), mando-motor,
  salidas-alarma, comunicacion, modos y alarma-temperatura reemplazados por delta
  specs en el cambio, más `sistema`; sincronizados en el cierre.
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
- I5 (regulador simple y redundancias de I2), pedido por el usuario antes de la
  puesta en marcha, implementado y verificado por Claude; rama `integracion-i5`,
  base a5a888e. `uint16_t Procesamiento_Regular(consigna_rpm, rpm_milesimas)` con la
  integral en memoria estática y las ganancias de `Control.h`; la cuenta del PI no
  cambia. Se retiran inicializar/reiniciar, instancia, configuración en ejecución,
  estados de resultado, retención de mediciones, `Medicion_Velocidad_Lectura` y
  `scripts/verificar_b17.py`. El modo borra la integral (arranque y MANUAL);
  `main.c` ya no reinicia el PI. Host/san 29/29 y host 29/29; ARM 9/9 y 9/9; Doxygen
  58 funciones sin avisos; MCUXpresso Debug y Release 0 errores y 0 avisos (`.axf`
  18 204 B y 10 256 B de flash, 360 B de SRAM sin pila); 16 de 17 mutantes
  detectados, 1 equivalente. El contrato vigente de regulacion-velocidad (B17) queda
  reemplazado por RG1–RG5 en el cierre de B18.
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

Puesta en marcha en placa (2026-11-13/15): la comunicación serie pasó de UART0
(P0.2/P0.3) a UART3, TXD3 P0.0 y RXD3 P0.1, función 10, vector 24, por mal contacto
de los pines originales; `UART0_Init` se llama ahora `UART3_Init`. La salida del
motor pasó de PWM1.1 (P2.0, pin dañado) a PWM1.2 en P2.1, con el mando en MR2
(`PWM_MATCH_2`). Las menciones anteriores a UART0, P0.2/P0.3, P2.0 y PWM_MATCH_1
en este documento son historial de los bloques B13 y B15. Contratos vigentes en
`openspec/specs/uart-dispositivo` y `openspec/specs/motor-dispositivo`.

## Cambios del 2026-11-16 (rama `sensor-lm335`, sin fusionar a main)

Hechos por Claude a pedido explícito del usuario, sin cambio SDD propio; contratos
vigentes editados directamente en `openspec/specs/`.

- Sensor de temperatura: LM335 en lugar del LM35 (no se consiguió), mismo
  AD0.1/P0.24. Dispositivo `Dispositivos/LM335.h/.c`; la lectura se llama
  `getTemperatura` y resta 273,15 K más `LM335_AJUSTE_CENTESIMAS`
  (`Configuracion/Analogica.h`). Contrato: `openspec/specs/temperatura`, TE6.
- Ventana y período del lazo: `TPI_CONTROL_PERIODO_MS` = 1000 (antes 500). Telemetría,
  alarma, modos y paso del PI una vez por segundo; ganancias por paso sin cambio.
- Consigna máxima: `TPI_RPM_MAX` = 300 (antes 185); R14 actualizado.
- Verificación: host 31/31, ARM 9/9, Doxygen sin avisos; proyecto MCUXpresso
  recompilado. Nada de esto está probado en placa: faltan armar el LM335, calibrar el
  ajuste y volver a probar el modo automático con paso de 1 s.

## Cierre del firmware (2026-11-19)

**Firmware aprobado por el usuario y hardware terminado.** Versión final en `main`
(rama de trabajo `consigna-700`, que parte de `exp-500ms`). Cambios del 2026-11-19
hechos por Claude a pedido del usuario, sin cambio SDD propio; contratos vigentes
editados directamente en `openspec/specs/`.

- Ventana de conteo y período del lazo: `TPI_CONTROL_PERIODO_MS` = 500 (queda sin
  efecto el cambio a 1 s del 2026-11-16). Con el disco de 60 ranuras cada pulso vale
  2 RPM.
- Consigna máxima: `TPI_RPM_MAX` = 700 (antes 300).
- Regulador PI: `Control.h` pasa de Kp 4/1000 y Ki 1/1000 (oscilaba entre 0 y 100 %
  de PWM) a Kp 1/2000 y Ki 1/2500 por mil por milésima de RPM. Los denominadores
  admiten hasta 10000 y se acota `KP_NUM × KI_DEN`. El algoritmo de
  `Procesamiento_Regular` no cambia: usa la integral del paso anterior, por lo que
  equivale a un PI con proporcional 0,1 ‰/RPM, integral 0,4 ‰/RPM por paso y
  Ti = 0,125 s.
- Verificación: host 31/31, ARM 9/9, Doxygen sin avisos; MCUXpresso Debug y Release
  con 0 errores y 0 avisos.

Mediciones en placa (motor con disco de 60 ranuras, capturas de la interfaz de PC):

- Manual: unas 480 RPM a PWM 61 %, 537 a 78 % y 630 a 656 a 100 %. Desde parado el
  motor arranca recién con un 60 % de PWM; bajando sigue girando hasta un 10 a 20 %.
- Escalón 0 → 75 % de PWM con la llave, tres repeticiones: régimen de unas 547 RPM y
  constante de tiempo de unos 0,14 s. Pendiente de la curva estática entre 0,4 y
  1,3 RPM por cada por mil de mando.
- Automático con el PI nuevo: consignas 500, 600, 300, 150 y 100 alcanzadas y
  sostenidas, sin oscilación entre saturaciones. 0 → 500 en 2,5 s con 5 % de
  sobrepico; 500 → 600 en unos 10 s (PWM 93 a 95 %); 600 → 300 en 4,5 s; 0 → 150
  tarda 5,5 s en arrancar y pasa por 472 RPM antes de bajar, por la zona muerta.
  Ruido en régimen de ±20 a 30 RPM, igual que con PWM fijo.
- Sensor óptico, LM335, alarma (LED y buzzer), potenciómetro, llave de modo y UART
  probados por el usuario el 2026-11-18. Las fallas previas de la etapa de potencia
  y del LM335 eran las masas de la fuente y de la placa sin unir.

Hardware: circuito soldado y listo para la entrega (usuario, 2026-11-19). Corriente
del motor: 100 mA (dato del usuario).

Queda fuera del firmware: Doxygen final, diagramas, informe y paquete de entrega.
No se probaron consignas de 650 a 700 RPM; a 600 el mando ya está en 93 a 95 %.
`Adquisicion_ADC_DMA_ContarPares` no tiene llamador.

Numeración para la entrega (2026-11-19): `Sistema`, `Adquisicion_ADC_DMA` y `Reloj`
son ESW.2.1.18 a ESW.2.1.20, y las USW de cada ESW se renumeraron en forma corrida
desde 1 (58 en total). Sólo comentarios Doxygen. Tabla vigente en
[docs/doxygen/README.md](../doxygen/README.md).

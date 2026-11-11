# Sistema — contrato vigente

Origen R01/R10/R15 y anteproyecto. Dominio nuevo de B18.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: SI1 — Arranque de plataforma
`Servicios/Sistema.h/.c` MUST exponer `bool Sistema_Init(void)`. MUST llamar, en
orden, `Reloj_Init` y `Adquisicion_ADC_DMA_Init`, y esperar el primer par de muestras
consultando `Adquisicion_ADC_DMA_HayDatos` como máximo 2 000 000 de veces. MUST
devolver false si el DMA rechaza el canal (sin esperar) o si se agota el límite.

#### Scenario: S1 — Reloj, ADC/DMA y primer par (`i2.sistema`)
- Given dispositivos simulados con datos disponibles tras 0 o 1000 consultas.
- When se llama `Sistema_Init`.
- Then MUST llamar `Reloj_Init` y luego `Adquisicion_ADC_DMA_Init`, y devolver true
  en la primera consulta con datos.

#### Scenario: S2 — Fallo y espera acotada (`i2.sistema`)
- Given el DMA rechazado, o datos que nunca llegan, o que llegan en la consulta 2 000 000.
- When se llama `Sistema_Init`.
- Then MUST devolver false sin consultar; false tras exactamente 2 000 000 consultas;
  y true en el último caso.

### Requirement: SI2 — Capas
`Aplicacion/` MUST incluir sólo encabezados de `Aplicacion/`, `Servicios/` y
`Configuracion/`; `Servicios/` MUST NOT incluir encabezados de `Aplicacion/`. Cada
servicio incluye sus dispositivos. La excepción O5 (main como raíz de composición
con dispositivos) queda retirada.

#### Scenario: S3 — Control automático de capas (`i2.capas`)
- Given los `.c`/`.h` de `Aplicacion/` y `Servicios/`.
- When se ejecuta `scripts/verificar_capas.py` (host y ARM).
- Then MUST fallar si Aplicación incluye un encabezado de `Dispositivos/` o uno no
  propio distinto de `stdbool.h`/`stdint.h`/`stddef.h`.

### Requirement: SI3 — Inicialización de la aplicación
`Aplicacion/main.h/.c` MUST exponer `bool Aplicacion_Inicializar(void)` y
`void Aplicacion_Iterar(void)`; `main()` MUST sólo encadenarlas. Inicializar MUST
ejecutar, en orden: `Sistema_Init`; `Driver_Motor_Init` y mando 0;
`Driver_Alarmas_Init` y alarma apagada; `Mandos_Init`; `Comunicacion_Init`; modo
inicial según la llave; regulador PI con `Control.h`; `Medicion_Velocidad_Init`
al final. Si `Sistema_Init` falla MUST dejar el motor en 0, encender la alarma,
no inicializar el resto y devolver false; `main()` MUST quedar detenido.

#### Scenario: S4 — Arranque en automático (`i2.aplicacion` S1)
- Given llave cerrada (AUTOMÁTICO), 25,00 °C y potenciómetro al máximo.
- When se inicializa, se itera sin ventana y luego se cierra una ventana.
- Then el orden de inicialización de dispositivos MUST ser Reloj, ADC/DMA, Motor,
  LED, Buzzer, Selector, UART0, Sensor óptico, Timer_Init, Timer_Start; mando 0,
  alarma apagada, sin tramas entre ventanas y trama
  `RPM=0.000;TEMP=25.00;ESTADO=AUTO_ESPERA\n`.

#### Scenario: S5 — Fallo de arranque (`i2.aplicacion` S8)
- Given ADC/DMA que rechaza el canal.
- When se inicializa.
- Then MUST devolver false con Motor/LED/Buzzer inicializados, mando 0 y alarma
  encendida, sin inicializar Selector, UART ni medición.

### Requirement: SI4 — Lazo cooperativo
En cada vuelta, con estado MANUAL, Iterar MUST aplicar al motor
`Mandos_LeerManual()`. Cuando `Medicion_Velocidad_Actualizar` entrega una medición
(cada 500 ms) MUST ejecutar, en orden: (1) alarma: `Temperatura_Leer` →
`Procesamiento_EvaluarAlarma` → `Driver_Alarmas_Aplicar`; (2) modo:
`Mandos_LeerModo`, todas las consignas pendientes (vale la última) y
`Procesamiento_ActualizarModo`; al pasar de MANUAL a AUTO_ESPERA,
`Comunicacion_Descartar` (la integral del PI ya la borra el modo MANUAL, contrato
modos MO2); (3) mando: MANUAL → potenciómetro; AUTO_ESPERA → 0; AUTO_ACTIVO →
`Procesamiento_Regular(Procesamiento_Consigna(), medición)`;
(4) una trama de telemetría. MUST NOT esperar ni bloquear.

#### Scenario: S6 — Consigna y regulación (`i2.aplicacion` S1)
- Given AUTO_ESPERA y ganancias por defecto (Kp 4/1000, Ki 1/1000).
- When llega `RPM 100\n` y se cierran ventanas con 0 y luego 50 pulsos.
- Then estado AUTO_ACTIVO con mando 400 y luego 100; trama
  `RPM=100.000;TEMP=25.00;ESTADO=AUTO_ACTIVO\n`. La consigna MUST NOT aplicarse
  antes de cerrar la ventana.

#### Scenario: S7 — Manual entre periodos (`i2.aplicacion` S3)
- Given estado MANUAL.
- When el potenciómetro vale 0, 2048 y 4095 en vueltas sin ventana.
- Then el mando MUST ser 0, 500 y 1000 en la misma vuelta, sin tramas; las
  consignas recibidas MUST ignorarse y la trama informar `ESTADO=MANUAL`.

#### Scenario: S8 — Cambio de modo (`i2.aplicacion` S2 y S4)
- Given MANUAL, o AUTO_ACTIVO con integral acumulada.
- When la llave pasa a AUTOMÁTICO con `RPM 100\n` y una línea parcial `RPM 1`
  pendientes, y después llega `20\n`; o se va a MANUAL y se vuelve.
- Then MUST quedar en AUTO_ESPERA con mando 0 hasta una consigna posterior al
  cambio; con ella el primer mando MUST ser 400 (PI reiniciado).

#### Scenario: S9 — Alarma independiente del modo (`i2.aplicacion` S5)
- Given sensor sostenido 4 periodos en 45,00 / 45,01 / 44,00 / 43,04 / 43,00 / 45,00 °C.
- When se cierran las ventanas.
- Then LED y buzzer MUST encender recién con el promedio 45,01, seguir encendidos
  hasta que el promedio llega a 43,00 y no volver a encender en 45,00; el motor
  MUST seguir su mando con la alarma activa.

#### Scenario: S10 — Consignas inválidas, cero y saturación (`i2.aplicacion` S6 y S7)
- Given AUTO_ESPERA.
- When llegan `RPM 186`, `RPM x`, una línea larga, `RPM 0`, varias en un periodo y
  `RPM 185` sostenida con el eje detenido.
- Then las inválidas MUST ignorarse; `RPM 0` MUST dar AUTO_ACTIVO con mando 0;
  MUST valer la última válida del periodo; el mando MUST llegar a 1000 sin superarlo.

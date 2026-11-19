# Comunicación — contrato vigente

Origen: R09, R14 y soporte de R06. Contratos anteriores en el historial archivado.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
funcionamiento en placa aceptado por el usuario el 2026-11-19 (prueba funcional
del sistema completo; ver `docs/agente/ESTADO.md`).
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: CO1 — Recepción de consignas
`Servicios/Comunicacion` MUST exponer `void Comunicacion_Init(void)` (llama
`UART3_Init` y vacía la línea) y `bool Comunicacion_RecibirConsigna(uint32_t *rpm)`.
Ésta MUST consumir bytes con `UART_Read` y devolver true con cada línea completa
`RPM`, un espacio, 1 a 3 dígitos decimales, CR opcional y LF, con valor 0..700
(`TPI_RPM_MAX`); ceros a la izquierda permitidos. Toda otra línea, incluida la de
más de 8 bytes antes del LF, MUST descartarse entera sin afectar a la siguiente.
MUST entregar una consigna por llamada, en orden, conservar la línea parcial entre
llamadas y devolver false, sin escribir el destino, cuando no quedan bytes.

#### Scenario: S1 — Consignas válidas (`i2.comunicacion`)
- Given `RPM 0`, `RPM 7`+CRLF, `RPM 100`, `RPM 699`, `RPM 700`+CRLF y `RPM 007`.
- When se recibe cada una.
- Then MUST entregar 0, 7, 100, 699, 700 y 7, una vez cada una.

#### Scenario: S2 — Líneas inválidas (`i2.comunicacion`)
- Given `RPM 701`, `RPM 999`, `RPM 1000`, `RPM x`, `RPM 1x`, `RPM `, `RPM`,
  dos espacios, minúsculas, `RPM=10`, `RPM -1`, espacio final o inicial, línea
  vacía, CR solo, doble CR y líneas largas.
- When se recibe cada una seguida de `RPM 42`.
- Then la inválida MUST NOT entregar consigna y `RPM 42` MUST entregarse.

#### Scenario: S3 — Línea partida y varias (`i2.comunicacion`)
- Given una consigna que llega en tres tramos, y `RPM 10`, `RPM 701`, `RPM 20` y
  un tramo parcial en una sola ráfaga.
- When se llama repetidamente.
- Then MUST entregar 123 al completarse; luego 10 y 20, y completar la parcial después.

### Requirement: CO2 — Descarte
MUST exponer `void Comunicacion_Descartar(void)`, que vacía la línea parcial y
retira sin interpretar todos los bytes pendientes de `UART_Read`.

#### Scenario: S4 — Descartar (`i2.comunicacion`)
- Given una línea parcial `RPM 1`, o `RPM 60\nRPM 7` sin procesar.
- When se descarta y luego llegan `50\n` o `0\n`.
- Then MUST NOT entregarse ninguna consigna; una línea completa posterior MUST entregarse.

### Requirement: CO3 — Telemetría
MUST exponer `bool Comunicacion_EnviarTelemetria(uint32_t rpm_milesimas, int32_t
temperatura_centesimas, uint16_t mando_por_mil, Comunicacion_Estado estado)`. MUST entregar a `UART_Write`
exactamente `RPM=<n>.<3 dígitos>;TEMP=[-]<n>.<2 dígitos>;PWM=<n>;ESTADO=MANUAL|
AUTO_ESPERA|AUTO_ACTIVO\n` (formato B09 más el campo `PWM`, agregado el 2026-11-18),
sin ceros a la izquierda en la parte entera, sin NUL, de 61 bytes como máximo.
`PWM` es el ciclo de trabajo entero de 0 a 100 %: `(min(mando, 1000) + 5) / 10`. MUST devolver true sólo si la
UART aceptó la trama completa. Sin float ni libc. `ND`/`ERR` dejan de existir.

#### Scenario: S5 — Trama exacta (`i2.comunicacion`)
- Given (rpm; temperatura; mando; estado): (0; 0; 0; MANUAL), (100000; 2500; 1000;
  AUTO_ACTIVO), (2000; 4501; 4; AUTO_ESPERA), (185007; 5; 500), (1; −50; 5),
  (12340; −1234; 994), (12340; −1234; 995), (0; 0; 1001), (0; 0; UINT16_MAX),
  (UINT32_MAX; INT32_MIN; 1000) y (0; INT32_MAX; 0).
- When se envía.
- Then MUST escribirse `RPM=0.000;TEMP=0.00;PWM=0;ESTADO=MANUAL\n`,
  `RPM=100.000;TEMP=25.00;PWM=100;ESTADO=AUTO_ACTIVO\n`,
  `RPM=2.000;TEMP=45.01;PWM=0;ESTADO=AUTO_ESPERA\n`, `…PWM=50…`, `…PWM=1…`, `…PWM=99…`,
  `…PWM=100…` (tres veces: 995, 1001 y UINT16_MAX),
  `RPM=4294967.295;TEMP=-21474836.48;PWM=100;ESTADO=AUTO_ACTIVO\n` (61 bytes) y
  `RPM=0.000;TEMP=21474836.47;PWM=0;…`.

#### Scenario: S6 — Sin lugar en transmisión (`i2.comunicacion`)
- Given una UART que acepta 39 o 40 bytes y una trama de 40.
- When se envía.
- Then MUST devolver false con 39 y true con 40.

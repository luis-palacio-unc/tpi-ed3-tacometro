# Delta B18/I2 — comunicacion (reemplaza el contrato vigente)

Candidato de la vía rápida B18. Los requisitos de B02 (recepción con cola y orden
de 64 bits) y CT1–CT4 de B09 (codificación con ND/ERR y buffer externo) quedan como
historial. Se conservan la gramática de la consigna y el formato de la trama.

## REMOVED Requirements

### Requirement: contrato B02 y B09 completo
Motivo: contexto público, cola FIFO de 4 eventos, orden de recepción uint64, nueve
resultados y validez ND/ERR por magnitud. El servicio pasa a usar la UART y la
aplicación procesa los bytes en orden.

## ADDED Requirements

### Requirement: CO1 — Recepción de consignas
`Servicios/Comunicacion` MUST exponer `void Comunicacion_Init(void)` (llama
`UART0_Init` y vacía la línea) y `bool Comunicacion_RecibirConsigna(uint32_t *rpm)`.
Ésta MUST consumir bytes con `UART_Read` y devolver true con cada línea completa
`RPM`, un espacio, 1 a 3 dígitos decimales, CR opcional y LF, con valor 0..185
(`TPI_RPM_MAX`); ceros a la izquierda permitidos. Toda otra línea, incluida la de
más de 8 bytes antes del LF, MUST descartarse entera sin afectar a la siguiente.
MUST entregar una consigna por llamada, en orden, conservar la línea parcial entre
llamadas y devolver false, sin escribir el destino, cuando no quedan bytes.

#### Scenario: S1 — Consignas válidas (`i2.comunicacion`)
- Given `RPM 0`, `RPM 7`+CRLF, `RPM 100`, `RPM 184`, `RPM 185`+CRLF y `RPM 007`.
- When se recibe cada una.
- Then MUST entregar 0, 7, 100, 184, 185 y 7, una vez cada una.

#### Scenario: S2 — Líneas inválidas (`i2.comunicacion`)
- Given `RPM 186`, `RPM 999`, `RPM 1000`, `RPM x`, `RPM 1x`, `RPM `, `RPM`,
  dos espacios, minúsculas, `RPM=10`, `RPM -1`, espacio final o inicial, línea
  vacía, CR solo, doble CR y líneas largas.
- When se recibe cada una seguida de `RPM 42`.
- Then la inválida MUST NOT entregar consigna y `RPM 42` MUST entregarse.

#### Scenario: S3 — Línea partida y varias (`i2.comunicacion`)
- Given una consigna que llega en tres tramos, y `RPM 10`, `RPM 300`, `RPM 20` y
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
temperatura_centesimas, Comunicacion_Estado estado)`. MUST entregar a `UART_Write`
exactamente `RPM=<n>.<3 dígitos>;TEMP=[-]<n>.<2 dígitos>;ESTADO=MANUAL|AUTO_ESPERA|
AUTO_ACTIVO\n` (formato B09 con datos válidos), sin ceros a la izquierda en la
parte entera, sin NUL, de 53 bytes como máximo. MUST devolver true sólo si la
UART aceptó la trama completa. Sin float ni libc. `ND`/`ERR` dejan de existir.

#### Scenario: S5 — Trama exacta (`i2.comunicacion`)
- Given (0; 0; MANUAL), (100000; 2500; AUTO_ACTIVO), (2000; 4501; AUTO_ESPERA),
  (185007; 5), (1; −50), (12340; −1234), (UINT32_MAX; INT32_MIN) y (0; INT32_MAX).
- When se envía.
- Then MUST escribirse `RPM=0.000;TEMP=0.00;ESTADO=MANUAL\n`,
  `RPM=100.000;TEMP=25.00;ESTADO=AUTO_ACTIVO\n`,
  `RPM=2.000;TEMP=45.01;ESTADO=AUTO_ESPERA\n`, `RPM=185.007;TEMP=0.05;…`,
  `RPM=0.001;TEMP=-0.50;…`, `RPM=12.340;TEMP=-12.34;…`,
  `RPM=4294967.295;TEMP=-21474836.48;ESTADO=AUTO_ACTIVO\n` (53 bytes) y
  `RPM=0.000;TEMP=21474836.47;…`.

#### Scenario: S6 — Sin lugar en transmisión (`i2.comunicacion`)
- Given una UART que acepta 33 o 34 bytes y una trama de 34.
- When se envía.
- Then MUST devolver false con 33 y true con 34.

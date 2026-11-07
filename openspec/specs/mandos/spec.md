# Mandos — contrato vigente

Origen: R03, contrato de modos B01 y portabilidad R15. B04 aceptado el 2026-11-06.
Sólo normalización lógica del mando manual; aceptación física pendiente.

## Requirements

### Requirement: MA1 — Normalización independiente de resolución

Mandos MUST recibir lectura y fondo de escala enteros uint32_t, con escala>0
y lectura<=escala. MUST entregar un mando uint16_t de 0..1000 por mil, proporcional
a lectura/escala, monótono, redondeado al más cercano con empate hacia arriba.
Error aritmético MUST ser <=0,5 unidades por mil; no garantiza exactitud del ADC.
La aplicación MUST conservar el rango lógico B01 sin conocer bits, VREF o canales.

#### Scenario: S1 — Extremos y recorrido

- Given escala positiva, lecturas cero, fondo de escala y puntos intermedios.
- When se normalizan en orden creciente.
- Then MUST obtener 0 y 1000 en extremos, aproximadamente 500 en mitad y
  salidas no decrecientes dentro de 0..1000.

#### Scenario: S2 — Escalas equivalentes y redondeo

- Given 341/1023, 1365/4095, 682/1023, 2730/4095 y 1/16 como lectura/escala.
- When se normalizan, además de valores a ambos lados de un empate.
- Then MUST producir 333, 333, 667, 667 y 63 respectivamente; empate hacia arriba.

### Requirement: MA2 — Disponibilidad, errores y recuperación

Inicializar MUST producir SIN_DATOS/cero. Normalizar MUST escribir valor y estado
sin depender del contenido anterior. Escala cero MUST producir ERROR_ESCALA;
lectura>escala positiva, ERROR_LECTURA. Todo error con salida escribible MUST
invalidarla y poner cero, sin saturar ni retener un mando anterior como válido.
Una nueva entrada válida MUST recuperar el mando, incluso cero válido.
Salida NULL MUST rechazarse sin acceso: false al inicializar, ERROR_ARGUMENTO
al normalizar. Prioridad de error: argumento, escala y lectura, en ese orden.

#### Scenario: S3 — Ausencia y reinicio

- Given una salida nueva, válida o errónea.
- When se inicializa.
- Then MUST quedar SIN_DATOS/cero, distinto del mando cero válido de lectura cero.

#### Scenario: S4 — Invalidación y recuperación

- Given un mando válido anterior.
- When llega escala cero o lectura fuera de escala, seguida de entrada válida.
- Then MUST devolver el error correspondiente con cero, y luego recuperar valor/validez.

#### Scenario: S5 — Fronteras y argumentos

- Given salida NULL o entradas hasta UINT32_MAX, incluida escala mínima uno.
- When se inicializa/normaliza.
- Then MUST respetar prioridades; MAX/MAX → 1000 y 1/MAX → 0 válidos;
  MAX/(MAX-1) → ERROR_LECTURA, sin overflow ni división por cero.

### Requirement: MA3 — Aislamiento y compatibilidad

El servicio MUST ser C11 sin heap, flotantes, globals mutables ni acceso a hardware.
Cada salida MUST ser independiente y su acceso serializado por el llamador.
Sólo VALIDO habilita consumir el valor; no se evalúa frescura de captura.
MUST mantener el contrato B01 y evitar dependencias Servicios→Aplicacion.
MUST NOT implementar ADC/DMA, selector, antirrebote, filtrado, calibración, PWM,
regulación ni integración de main.

#### Scenario: S6 — Instancias y consumidor B01

- Given dos salidas independientes y Procesamiento en manual.
- When una salida se actualiza/reinicia/falla y se adaptan mandos válidos a B01.
- Then MUST conservar la otra salida y B01 entregar fuente MANUAL con el mismo mando.

### Requirement: MA4 — Verificación y documentación

B04 MUST pasar C host, sanitizadores, regresiones B00–B03, enlace ARM y Doxygen
estricto ESW/USW. MUST distinguir pruebas host y ELF de ejecución ARM/simulación/hardware.

#### Scenario: S7 — Evidencia reproducible

- Given implementación y documentación B04.
- When se ejecutan controles y se revisan diff y recursos.
- Then MUST registrar resultados reales; recorrido eléctrico y hardware quedan pendientes.

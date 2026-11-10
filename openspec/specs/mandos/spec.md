# Mandos — contrato vigente

Origen: R03/R05/R06/R10/R11/R15 y contrato de modos B01. B04 aceptado el
2026-11-06; B10 aceptado/archivado el 2026-11-09. Normalización manual y selección
lógica estable de modo; GPIO/base temporal/main y aceptación física pendientes.

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

Mandos MUST ser C11 sin heap, flotantes, globals mutables o hardware. Instancias
MUST ser independientes y serializadas externamente. Sólo VALIDO habilita consumo;
MUST conservar B04/B01 y evitar Servicios→Aplicacion. B10 MAY filtrar un selector
lógico con muestras/tiempo externos. MUST NOT implementar ADC/DMA, GPIO/selector
físico, reloj/ISR real, filtrado analógico, calibración, PWM, regulación o main.

#### Scenario: S6 — Instancias y consumidor B01
- Given mandos manuales B04 y selector lógico B10 independientes.
- When se actualizan/reinician y se adaptan salidas disponibles en pruebas.
- Then MUST preservar normalización y contratos B01/B08 sin hardware concreto.

### Requirement: MA4 — Verificación y documentación

B04 MUST pasar C host, sanitizadores, regresiones B00–B03, enlace ARM y Doxygen
estricto ESW/USW. MUST distinguir pruebas host y ELF de ejecución ARM/simulación/hardware.

#### Scenario: S7 — Evidencia reproducible

- Given implementación y documentación B04.
- When se ejecutan controles y se revisan diff y recursos.
- Then MUST registrar resultados reales; recorrido eléctrico y hardware quedan pendientes.

### Requirement: SE1 — Estabilidad observada

MUST inicializar contexto con intervalo uint32_t 0..INT32_MAX ms, sin valor TPI
impuesto, SIN_DATOS y modo NINGUNO. Primera muestra válida MUST iniciar candidato;
MUST confirmar sólo tras intervalo cumplido con muestras sin cambio de modo.
Cada cambio MUST reiniciar candidato/fecha. Mientras se califica otro modo MUST
conservar confirmado/VALIDO; antes de primera confirmación, SIN_DATOS/NINGUNO.
Cero MUST aceptar inmediatamente. Reiniciar MUST borrar toda confirmación previa.

#### Scenario: B10-S1 — Inicio y frontera
- Given selector nuevo y modo manual o automático repetido.
- When transcurre intervalo-1 y luego intervalo ms desde primera muestra.
- Then MUST seguir sin modo y después publicar el modo válido, respectivamente.

#### Scenario: B10-S2 — Rebotes y cero
- Given modo confirmado y muestras alternadas, o intervalo cero.
- When aparece nuevo candidato y cumple/reinicia su intervalo.
- Then MUST conservar confirmado durante rebotes; confirmar al límite; cero acepta sin espera.

### Requirement: SE2 — Tiempo modular

MUST recibir uint32_t ms externo; cada avance aceptado MUST ser <2^31 ms.
Marcas iguales y rollover MUST permitirse; delta modular>INT32_MAX MUST producir
ERROR_TIEMPO, invalidar modo/candidato y descartar anclaje temporal. Siguiente llamada
válida MUST iniciar época/candidato nuevos, sin restaurar modo anterior.
El llamador MUST garantizar cadencia; no se detectan vueltas completas omitidas.

#### Scenario: B10-S3 — Tiempo y recuperación
- Given marcas repetidas, cruce UINT32_MAX→0 o salto fuera de contrato.
- When se actualiza el selector y luego se recuperan muestras/tiempo.
- Then MUST respetar intervalo modular o informar error y exigir estabilidad completa nueva.

### Requirement: SE3 — Disponibilidad y errores

Fuentes SIN_DATOS/ERROR MUST invalidar confirmación/candidato con SIN_DATOS/ERROR_FUENTE,
ignorando modo. Fuente válida con modo distinto de MANUAL/AUTOMATICO MUST producir
ERROR_MODO; fuente desconocida, ERROR_ARGUMENTO. Configuración excesiva MUST fallar al
inicializar y mantener ERROR_CONFIGURACION hasta reinicio válido. NULL MUST rechazarse.
Prioridad MUST ser NULL, configuración, fuente desconocida, tiempo, fuente ausente/
fallida y modo inválido. Timestamp aceptado MUST conservarse ante fallos de fuente/modo;
fuente desconocida no actualiza tiempo. Todo error escribible MUST dejar modo NINGUNO.

#### Scenario: B10-S4 — Ausencia, fallo y argumentos
- Given modo disponible y fuente ausente/fallida, modo/fuente inválidos o NULL.
- When se actualiza y luego llegan muestras válidas.
- Then MUST invalidar con diagnóstico/prioridad correctos y exigir candidato nuevo.

#### Scenario: B10-S5 — Configuración e instancias
- Given intervalos 0/INT32_MAX/excesivos, reinicio o dos contextos.
- When se inicializan/actualizan.
- Then MUST validar configuración, borrar estado anterior y preservar la otra instancia.

### Requirement: SE4 — Composición y evidencia

Sólo modo VALIDO MUST adaptarse a B01 en pruebas; transición confirmada manual→auto
MUST solicitar cero mediante B08 y exigir consigna posterior. Rebotes MUST NOT
invalidar consignas activas. Indisponibilidad MUST conservar diagnóstico sin emitir
órdenes nuevas desde adaptador de prueba; política física de fallo queda pendiente.
MUST documentar USW y pasar host/san/regresiones/ARM/Doxygen; no acreditar estabilidad eléctrica.

#### Scenario: B10-S6 — Integración y verificación
- Given selector, B01/B08 y proveedores emulados, incluyendo rollover temporal.
- When se prueban rebotes/transiciones, órdenes y controles de compilación/documentación.
- Then MUST conservar contratos/orden B01, reportar evidencia por modalidad y no integrar main.

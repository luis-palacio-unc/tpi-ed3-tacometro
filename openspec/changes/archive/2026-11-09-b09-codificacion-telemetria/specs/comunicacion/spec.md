# Comunicación — candidato B09

Origen R09/R10/R11/R15; alcance y formato acordados el 2026-11-09.
Delta sobre comunicación vigente; aceptación del resultado todavía pendiente.

## MODIFIED Requirements

### Requirement B02-R5: Evidencia y USW

El código MUST compilar/enlazar en C11 host/ARM y documentar ESW/USW con Doxygen.
MUST preservar recepción B02. B09 MAY codificar telemetría lógica; MUST NOT acceder
a UART física, ISR, PWM, envío/colas TX, periodicidad o main de producción.
(Antes: B02 excluía toda telemetría de su alcance original.)

#### Scenario S9: Verificación
- Given recepción B02 y codificación B09 implementadas.
- When se ejecutan host/sanitizadores, enlace ARM, regresiones y Doxygen.
- Then MUST pasar sin confundir codificación/ejecución host con transmisión física.

## ADDED Requirements

### Requirement: CT1 — Formato y unidades

MUST codificar ASCII `RPM=<r>;TEMP=<t>;ESTADO=<e>\n`, sin espacios ni CR.
RPM válido recibe uint32_t milésimas, muestra tres decimales, sin recortar a 185.
Temperatura válida recibe int32_t centésimas Celsius, muestra dos decimales y
signo sólo si negativa. Parte entera sin ceros extra; fracciones con ceros iniciales.
ESTADO MUST ser MANUAL, AUTO_ESPERA o AUTO_ACTIVO. Longitud MUST incluir LF y excluir
NUL final. El máximo MUST ser 53 bytes de trama y 54 de almacenamiento con NUL.

#### Scenario: B09-S1 — Formato y estados
- Given 123456 milésimas, 2534 centésimas y estado MANUAL.
- When se codifica, incluyendo variantes de estado/decimales/cero.
- Then MUST producir `RPM=123.456;TEMP=25.34;ESTADO=MANUAL\n` y variantes exactas.

#### Scenario: B09-S2 — Fronteras numéricas
- Given UINT32_MAX y temperaturas INT32_MIN/MAX o -1 centésima.
- When se codifica con capacidad suficiente.
- Then MUST representar 4294967.295, -21474836.48, 21474836.47 o -0.01 exactamente, sin UB.

### Requirement: CT2 — Disponibilidad y estado

RPM/temperatura MUST tener validez propia: VALIDO, SIN_DATOS o ERROR.
MUST emitir ND/ERR para SIN_DATOS/ERROR, ignorando el campo numérico; cero válido
MUST permanecer 0.000/0.00. Estado/validez desconocidos MUST producir ERROR_ESTADO.
MUST recibir estados propios de comunicación, sin depender de Aplicacion/sensores.

#### Scenario: B09-S3 — Ausencia y fallo
- Given todas las combinaciones de disponibilidad de ambas lecturas.
- When se codifica.
- Then MUST distinguir números válidos, ND y ERR sin reutilizar medidas anteriores.

### Requirement: CT3 — Buffer, validación y recuperación

MUST recibir instantánea const, buffer/capacidad y longitud escribible no solapados.
Prioridad MUST ser punteros requeridos NULL, estados desconocidos y capacidad
insuficiente. Capacidad cero MUST producir ERROR_CAPACIDAD con punteros/estados válidos.
MUST devolver resultado completo, sin trama parcial utilizable. En error MUST
poner longitud cero si existe y buffer[0]=NUL si buffer existe/capacidad>0;
sin acceso fuera de capacidad ni modificación adicional de destino ante error.
El llamador MUST garantizar memoria válida/coherente y acceso serializado.

#### Scenario: B09-S4 — Argumentos y estados inválidos
- Given NULL o estado desconocido junto con capacidad insuficiente.
- When se codifica.
- Then MUST respetar prioridad y limpiar salidas escribibles sin acceso inválido.

#### Scenario: B09-S5 — Capacidades exactas
- Given trama de N bytes y destino protegido por canarios.
- When capacidad es 0..N o N+1.
- Then MUST rechazar 0..N limpiando sólo salidas permitidas; N+1 produce trama/NUL completos.

#### Scenario: B09-S6 — Recuperación e independencia
- Given buffers independientes y una solicitud errónea seguida de válida/repetida.
- When se codifican ambas.
- Then MUST recuperar, preservar entrada y el otro buffer, sin estado compartido.

### Requirement: CT4 — Composición y evidencia

MUST usar C11 sin heap, flotantes, esperas, configuración de MCU ni dependencia
Servicios→Aplicacion. Adaptación B01/B03/B05 MUST permanecer en pruebas/fixture.
MUST conservar contratos/pruebas RX, pasar host/san/ARM/Doxygen y documentar USW.

#### Scenario: B09-S7 — Integración lógica y recepción
- Given magnitudes B03/B05, estado B01 y recepción B02 parcial/con eventos.
- When se adaptan/codifican instantáneas.
- Then MUST obtener tramas correctas conservando FIFO, parser, órdenes y decisiones B01.

#### Scenario: B09-S8 — Evidencia reproducible
- Given implementación y controles anteriores preservados.
- When se ejecutan pruebas, enlace, documentación y revisión de recursos/diff.
- Then MUST registrar evidencia real; UART/main/transmisión física siguen pendientes.

# Comunicación — contrato vigente

Origen: R09, R14 y soporte de R06. Alcance autorizado el 2026-11-06.
B02 aceptado/archivado el 2026-11-06; B09 aceptado/archivado el 2026-11-09.
B09 agrega R10/R11/R15 y telemetría lógica; UART/transmisión física pendientes.

## Requirements

### Requirement B02-R1: Gramática y rango
El servicio MUST aceptar exactamente ASCII `RPM ` seguido de 1..3 dígitos decimales
y LF o CRLF; MAY incluir ceros iniciales. El valor MUST pertenecer a 0..185 RPM.
MUST NOT aplicar signos, espacios extra, fracciones, caracteres de control o saturación.

#### Scenario S1: Tramas válidas
- GIVEN tramas con 0/001/184/185, LF o CRLF
- WHEN se reciben completas
- THEN se produce exactamente un evento válido por trama, con su valor entero.

#### Scenario S2: Entrada inválida
- GIVEN negativos, 186/999, exceso de dígitos, texto, NUL u otros bytes no admitidos
- WHEN se completa o desborda su trama
- THEN error explícito y ningún evento aplicable, sin conversiones indefinidas.

### Requirement B02-R2: Recepción incremental y orden
El servicio MUST aceptar bytes fragmentados o consecutivos y MUST NOT emitir eventos
incompletos. Cada byte MUST llevar una secuencia global estrictamente creciente,
asignada externamente al recibirlo. El evento MUST conservar la marca del LF final;
extraerlo de la cola MUST NOT generar otra marca ni aceptar autoridad de la PC.

#### Scenario S3: Fragmentación y cola
- GIVEN una trama parcial o varias completas sin consumir
- WHEN se completan y extraen eventos
- THEN FIFO, sin duplicados; marcas originales del LF conservadas.

#### Scenario S4: Orden inválido
- GIVEN byte con secuencia repetida/regresiva, incluido rollover
- WHEN se recibe
- THEN error de orden, preservar último orden confiable y cola previa; descartar la
  trama afectada hasta LF y permitir recepción posterior correctamente ordenada.

### Requirement B02-R3: Límites y recuperación
El buffer MUST contener como máximo 8 bytes antes de LF (incluye CR opcional), y
la cola MUST contener 4 eventos. No heap. Un noveno byte no LF MUST señalar longitud
excesiva y descartar hasta LF. Cola llena MUST rechazar la nueva trama completa,
conservar FIFO previa y señalar error; no sobrescribir eventos.

#### Scenario S5: Trama demasiado larga
- GIVEN exceso de longitud seguido de un sufijo que parece comando
- WHEN se recibe hasta LF y luego otra trama válida
- THEN el sufijo no se interpreta; la trama válida siguiente se procesa normalmente.

#### Scenario S6: Cola llena
- GIVEN cuatro eventos pendientes
- WHEN llega un quinto, se libera espacio y llega otro
- THEN se rechaza el quinto y los anteriores se conservan; el siguiente se encola.

### Requirement B02-R4: API y capas
API MUST inicializar, recibir bytes y extraer eventos propios del servicio; MUST
informar argumentos nulos y cola vacía sin consumir datos indebidamente. Llamadas
MUST estar serializadas y el contexto inicializado; reiniciar purga cola/trama.
El límite RPM MUST tener una fuente común sin dependencia Servicios→Aplicacion.

#### Scenario S7: Errores y reinicio
- GIVEN cola vacía, parámetros NULL, dos contextos o un reinicio
- WHEN se invoca la API
- THEN resultados explícitos, sin accesos inválidos, mezcla de instancias o datos previos.

#### Scenario S8: Integración B01
- GIVEN evento completado antes de entrar en automático, extraído después
- WHEN la aplicación lo adapta a B01
- THEN no habilita automático; otra trama recibida después sí puede habilitarlo.

### Requirement B02-R5: Evidencia y USW

El código MUST compilar/enlazar en C11 host/ARM y documentar ESW/USW con Doxygen.
MUST preservar recepción B02. B09 MAY codificar telemetría lógica; MUST NOT acceder
a UART física, ISR, PWM, envío/colas TX, periodicidad o main de producción.

#### Scenario S9: Verificación
- Given recepción B02 y codificación B09 implementadas.
- When se ejecutan host/sanitizadores, enlace ARM, regresiones y Doxygen.
- Then MUST pasar sin confundir codificación/ejecución host con transmisión física.

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

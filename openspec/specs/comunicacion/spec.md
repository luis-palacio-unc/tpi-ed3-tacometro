# Comunicación — contrato vigente

Origen: R09, R14 y soporte de R06. Alcance autorizado el 2026-11-06.
Resultado aceptado el 2026-11-06. Origen B02 archivado; complementa build y modos.

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
MUST NOT acceder a UART física, ISR, PWM, telemetría o main de producción.

#### Scenario S9: Verificación
- GIVEN implementación y pruebas
- WHEN se ejecutan C host/sanitizadores, enlace ARM, regresiones y Doxygen estricto
- THEN pasan; la evidencia distingue ejecución host de verificación estática ARM.

# Modos — contrato candidato B01

Origen: R05/R06 y R14 (aclaración del usuario: máximo 185 RPM); interfaces de R03/R04. Alcance autorizado el 2026-11-06.
Sin aceptación del resultado todavía; no reemplaza el contrato build vigente.

## Requirements

### Requirement B01-R1: Selección exclusiva
Procesamiento MUST producir exactamente una fuente lógica: CERO, MANUAL o AUTOMATICA.
En manual MUST entregar sólo el mando normalizado del potenciómetro e ignorar consignas.
En automático MUST ignorar completamente el mando manual, incluso fuera de rango.

#### Scenario S1: Manual
- GIVEN arranque o selección manual
- WHEN se actualiza el mando dentro de 0..1000 por mil, con o sin consigna
- THEN fuente MANUAL, mando correspondiente y consigna de salida cero.

#### Scenario S2: Automático activo
- GIVEN una nueva consigna válida aceptada
- WHEN varía el mando manual o no llega otra consigna
- THEN fuente AUTOMATICA y misma consigna; mando manual de salida cero.

### Requirement B01-R2: Entrada en automático
Al arrancar o entrar en automático, el sistema MUST invalidar la consigna anterior
y producir fuente CERO hasta una nueva consigna válida recibida después de la entrada.
El orden MUST corresponder a recepción, no extracción de cola. Si cambio de modo y
consigna se observan en la misma actualización, MUST prevalecer la espera.

#### Scenario S3: Arranque y transición
- GIVEN arranque automático o actualización que cambia manual→automático
- WHEN no existe consigna válida posterior, o viene en la misma actualización
- THEN AUTO_ESPERA_CONSIGNA y fuente CERO.

#### Scenario S4: Consigna nueva
- GIVEN espera con límite de entrada registrado
- WHEN llega consigna válida posterior al límite y no futura
- THEN AUTO_ACTIVO con fuente AUTOMATICA y RPM recibidas.

#### Scenario S5: Retorno y reentrada
- GIVEN auto activo, retorno a manual y otra entrada en automático
- WHEN se procesa una consigna de la entrada anterior o recibida en manual
- THEN permanece esperando; sólo otra consigna nueva habilita automático.

### Requirement B01-R3: Rechazo sin sustituir
Consignas inválidas, anteriores/iguales al límite, duplicadas, fuera de orden o futuras
MUST NOT activar automático ni sustituir una consigna activa. Las consignas MUST
pertenecer a 0..185 RPM inclusive; valores superiores MUST descartarse aunque estén
marcados válidos externamente. El dato de entrada es uint32_t; no se satura a 185.

#### Scenario S6: Mensajes descartados
- GIVEN espera o auto activo
- WHEN se reciben eventos inválidos, fuera de 0..185 RPM, antiguos, repetidos, desordenados o futuros
- THEN se conserva espera o la consigna activa, respectivamente.

### Requirement B01-R4: Errores y límites C
El orden lógico MUST ser uint64_t monotónico sin rollover. Modo inválido, tiempo que
retrocede o mando manual >1000 en manual MUST fallar con salida CERO y exigir nueva
entrada lógica/consigna para recuperar automático. Parámetros nulos MUST fallar;
si existe salida, MUST devolverla a CERO. Contextos MUST estar inicializados y
llamadas/datos de recepción serializados por el integrador; no hay acceso concurrente.

#### Scenario S7: Entradas erróneas
- GIVEN contexto inicializado y una salida escribible
- WHEN modo, mando manual, orden o punteros son inválidos
- THEN error sin usar el mando anterior; recuperación automática exige nueva consigna.

#### Scenario S8: Fronteras y aislamiento
- GIVEN órdenes hasta UINT64_MAX, entradas RPM 0/184/185/186/UINT32_MAX e instancias independientes
- WHEN se actualizan sin rollover
- THEN sólo se aceptan RPM 0..185, sin desbordes, contaminación ni mezcla de fuentes.

### Requirement B01-R5: Evidencia y documentación
El módulo MUST compilar en C11 para host/ARM y documentar ESW/USW con Doxygen,
parámetros, unidades, errores y concurrencia. MUST NOT accionar hardware o calcular PI/PID.

#### Scenario S9: Verificación del bloque
- GIVEN módulo y pruebas
- WHEN se ejecutan pruebas C host, build/enlace ARM, pruebas B00 y Doxygen
- THEN pasan; queda explícito que no acreditan funcionamiento físico.

# Mando de motor — candidato B08

Origen R03/R06/R10/R11/R15 y alcance acordado el 2026-11-09.
Dominio nuevo; no modifica contratos B01/B04 ni constituye aceptación física.

## Requirements

### Requirement: MM1 — Mando normalizado y cero explícito

Driver_Motor MUST recibir mando uint16_t 0..1000 por mil inclusivo, con límite
compartido: 0 representa 0 % y 1000, 100 %. MUST enviar el valor exacto una vez
por solicitud válida mediante una salida reemplazable y contexto externo.
Solicitar cero MUST emitir 0 bajo el mismo contrato, sin init, cache o estado
global mutable. No presupone detención del eje ni confirma PWM físico.

#### Scenario: S1 — Dominio y repetición
- Given salida emulada completa con contador/último mando.
- When recibe 0, valores intermedios, 1000 o solicitudes repetidas.
- Then MUST enviar exactamente cada valor, una llamada por solicitud.

#### Scenario: S2 — Solicitud de cero
- Given una salida que previamente recibió un mando no nulo.
- When se solicita cero, incluso repetidamente.
- Then MUST enviar 0 una vez por solicitud y reportar aceptación/fallo actual.

### Requirement: MM2 — Validación, fallo y recuperación

Interfaz NULL o función ausente MUST devolver ERROR_ARGUMENTO antes de validar
rango. Mando>1000 MUST devolver ERROR_RANGO, sin saturar. Ambos errores MUST
evitar cualquier llamada; rechazar no equivale a solicitar cero. Contexto NULL
MUST permitirse si el proveedor lo admite. Proveedor true MUST producir ACEPTADA;
false, ERROR_SALIDA. Sin reintentos ni retención de error; cada llamada reemplaza
el resultado lógico anterior, sin garantizar efecto eléctrico ante fallo.

#### Scenario: S3 — Argumentos y fronteras
- Given interfaz nula/incompleta o valores 1001..UINT16_MAX.
- When se aplica un mando o se solicita cero con interfaz inválida.
- Then MUST respetar prioridad y no invocar salida ni reemplazar el último mando emitido.

#### Scenario: S4 — Fallos recuperables
- Given proveedor que falla y luego vuelve a aceptar.
- When se aplica mando o se solicita cero antes/después de recuperar.
- Then MUST reportar ERROR_SALIDA/ACEPTADA, una llamada cada vez sin reintento.

#### Scenario: S5 — Contextos e interfaz
- Given interfaces independientes, configuración const y proveedor que admite contexto NULL.
- When se alternan solicitudes entre instancias.
- Then MUST preservar configuración, pasar contexto exacto y no intercambiar estado.

### Requirement: MM3 — Portabilidad y composición

El servicio MUST ser C11, sin heap, esperas propias, registros de MCU ni dependencia
de Aplicacion/Dispositivos concretos. El llamador MUST mantener interfaz/contexto
válidos y estables y serializar acceso; proveedores MUST retornar sin espera activa.
Adaptación B04/B01 MUST permanecer en pruebas: MANUAL usa sólo mando válido,
CERO solicita cero y AUTOMATICA no envía consigna RPM como PWM. B01/B04 intactos.

#### Scenario: S6 — Manual y entrada automática
- Given mando válido B04 y selección B01 en manual.
- When se aplica manual y luego entra en automático/espera de nueva consigna.
- Then MUST emitir el mando manual y después cero, conservando contratos B01/B04.

#### Scenario: S7 — Valores no consumibles
- Given mando B04 no válido o fuente AUTOMATICA con consigna RPM válida.
- When la adaptación de prueba evalúa esas entradas.
- Then MUST evitar consumir mando inválido o enviar RPM como ciclo de trabajo.

### Requirement: MM4 — Evidencia y límites

B08 MUST pasar host/sanitizadores, regresiones, enlace ARM y Doxygen ESW/USW.
MUST NOT incorporar PWM real/pines/frecuencia/etapa de potencia, regulador, rampas,
inversión/frenado, adquisición, UART, alarmas o integración de main.

#### Scenario: S8 — Verificación reproducible
- Given implementación, proveedores emulados y controles previos preservados.
- When se ejecutan pruebas, enlace, documentación y revisión de recursos/diff.
- Then MUST registrar evidencia lógica separada de ejecución ARM y pruebas físicas pendientes.

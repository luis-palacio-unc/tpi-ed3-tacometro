# Salidas de alarma — contrato candidato B07

Origen R07/R10/R11/R15; alcance acordado 2026-11-09, resultado pendiente de aceptación.

## Requirements

### Requirement: SA1 — Solicitudes coordinadas

Driver_Alarmas MUST recibir activar/desactivar y una interfaz explícita de dos
funciones de salida con contextos externos. Activar MUST solicitar true a ambas;
desactivar, false. Interfaz válida MUST invocar LED y después buzzer, exactamente
una vez por canal/llamada, aun cuando LED informe fallo. No asumir atomicidad eléctrica.

#### Scenario: S1 — Activar, desactivar y repetir
- Given dos salidas emuladas y una interfaz completa.
- When se activan, desactivan o repiten solicitudes.
- Then MUST entregar el mismo estado lógico a ambas, una llamada por canal sin cache.

#### Scenario: S2 — Orden e instancias
- Given dos interfaces con contextos independientes y registro de llamadas.
- When se alternan solicitudes.
- Then MUST preservar LED→buzzer por llamada y no intercambiar contextos/estado entre instancias.

### Requirement: SA2 — Validación y errores observables

Interfaz NULL o cualquiera de sus funciones NULL MUST devolver ERROR_ARGUMENTO
antes de invocar ninguna salida. Contextos NULL MUST permitirse si el proveedor
los admite. Funciones retornan bool de aceptación; ambas true MUST producir ACEPTADA;
sólo LED false, ERROR_LED; sólo buzzer false, ERROR_BUZZER; ambas false,
ERROR_AMBOS. MUST NOT declarar aceptación total ante fallo parcial ni reintentar.
Cada llamada MUST devolver resultado completo sin retener fallos previos.

#### Scenario: S3 — Interfaz incompleta
- Given NULL o funciones ausentes, incluida LED válida y buzzer NULL.
- When se aplica una solicitud.
- Then MUST producir ERROR_ARGUMENTO sin llamadas ni efectos en contextos.

#### Scenario: S4 — Fallos y recuperación
- Given las cuatro combinaciones de aceptación de los proveedores.
- When se emite solicitud y luego se recuperan las salidas.
- Then MUST informar el canal fallido/ambos, intentar los dos y recuperar con ACEPTADA.

#### Scenario: S5 — Contextos y configuración
- Given contextos NULL admitidos por sus funciones y una interfaz const.
- When se aplica una solicitud.
- Then MUST funcionar sin alterar funciones/contextos de la interfaz; efectos pertenecen al proveedor.

### Requirement: SA3 — Portabilidad e integración

El servicio MUST ser C11 sin heap, globals mutables, reloj, esperas, GPIO o modelos
concretos. MUST NOT depender de Aplicacion, temperatura o umbral. El llamador MUST
mantener interfaz/contextos válidos y estables, serializar acceso y proveer funciones
que retornen. Adaptación B06 MUST permanecer fuera del servicio: sólo ACTIVA/INACTIVA
generan solicitudes. Ausencia/error MUST conservar diagnóstico sin emitir apagado u otra orden.

#### Scenario: S6 — Decisiones evaluables B06
- Given lectura genérica B05 y evaluación B06 activa/inactiva.
- When se adapta a activar/desactivar.
- Then MUST emitir solicitudes correctas sin dependencia del servicio hacia aplicación/dispositivos.

#### Scenario: S7 — Ausencia/error de B06
- Given alarma previamente solicitada activa y después lectura ausente/errónea.
- When B06 produce decisión no evaluable.
- Then MUST conservar diagnóstico, no invocar salidas y no declarar una nueva aplicación exitosa.

### Requirement: SA4 — Evidencia y límites

B07 MUST pasar host/sanitizadores, regresiones, enlace ARM y Doxygen ESW/USW.
MUST NOT incorporar GPIO/polaridad, patrón físico de fallo, histéresis, enclavamiento,
reintentos, timeout, prioridad temporal, motor, adquisición, UART o main. ACEPTADA
MUST significar aceptación por proveedores, sin acreditar estado eléctrico real.

#### Scenario: S8 — Verificación
- Given código nuevo, proveedores emulados y controles anteriores preservados.
- When se ejecutan pruebas, ELF, documentación y revisión del diff.
- Then MUST registrar evidencia lógica separada de compilación ARM y aceptación física pendiente.

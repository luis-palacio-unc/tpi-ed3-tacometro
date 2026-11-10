# Delta candidato B07-R1 — salidas de alarma

Origen R07/R10/R11/R15 y corrección usuario2026-11-10. Alcance acordado;
no sustituye el contrato vigente hasta implementar, verificar y aceptar el resultado.

## MODIFIED Requirements

### Requirement: SA1 — Solicitudes coordinadas

Driver_Alarmas MUST recibir activar y una interfaz de dos funciones void(bool).
MUST eliminar los contextos. Interfaz válida MUST invocar LED y después buzzer,
exactamente una vez cada uno con el mismo bool, sin caché/reintentos/atomicidad física.
Antes: funciones bool(void*,bool) con aceptación y contextos externos.

#### Scenario: S1 — Activar, desactivar y repetir
- Given dos salidas void(bool) y una interfaz completa.
- When se emiten true/false y órdenes repetidas.
- Then MUST transmitir el mismo bool una vez por canal, LED→buzzer cada vez.

#### Scenario: S2 — Interfaces independientes
- Given dos pares de funciones distintos y registros de llamada.
- When se alternan sus interfaces.
- Then MUST actuar únicamente el par solicitado sin retener configuración.

### Requirement: SA2 — Validación y errores observables

Interfaz NULL o cualquiera de sus funciones NULL MUST producir ERROR_ARGUMENTO
antes de cualquier llamada. Interfaz válida MUST producir ACEPTADA tras retornar
ambas funciones. MUST conservar sólo esos dos estados; eliminar ERROR_LED,
ERROR_BUZZER y ERROR_AMBOS. ACEPTADA MUST significar solicitud despachada, no
confirmación eléctrica. MUST NOT leer GPIO ni inventar diagnósticos de proveedor.
Antes: clasificación de fallos/contextos que estas salidas no informan/necesitan.

#### Scenario: S3 — Interfaz incompleta
- Given NULL o una/ambas funciones ausentes, incluso LED válida y buzzer ausente.
- When se aplica una solicitud.
- Then MUST obtener ERROR_ARGUMENTO y cero llamadas a ambas salidas.

#### Scenario: S4 — Recuperación tras argumento inválido
- Given una llamada inválida seguida de interfaz válida.
- When se aplica la siguiente solicitud.
- Then MUST invocar ambas salidas y obtener ACEPTADA sin error retenido.

#### Scenario: S5 — Configuración inmutable
- Given una interfaz const estable y funciones conformes.
- When se aplica una solicitud.
- Then MUST conservar ambos punteros sin almacenar ni liberar la interfaz.

### Requirement: SA3 — Portabilidad e integración

El servicio MUST conservar C11 sin heap/globals mutables/reloj/esperas ni dependencia
de Aplicacion/Dispositivos/GPIO/MCU. Llamador MUST mantener interfaz/funciones válidas
y estables, serializar acceso y suministrar funciones que retornen. Adaptación B06
MUST permanecer externa: sólo ACTIVA/INACTIVA generan órdenes; ausencia/error conserva
diagnóstico sin emitir apagado. Funciones B12 MUST ser utilizables directamente,
sin adaptadores/casts. Antes: exigía además contextos válidos externos.

#### Scenario: S6 — Decisiones evaluables B06
- Given lectura/umbral producen ACTIVA o INACTIVA.
- When el consumidor adapta esa decisión.
- Then MUST emitir true/false a ambas salidas sin dependencia inversa del servicio.

#### Scenario: S7 — Ausencia/error de B06
- Given alarma previamente solicitada activa y luego lectura ausente/errónea.
- When la decisión no es evaluable.
- Then MUST conservar diagnóstico sin llamar salidas ni afirmar nueva aplicación.

#### Scenario: S9 — Composición directa con B12
- Given interfaz con LED_Write/Buzzer_Write y doble host de API GPIO.
- When Driver_Alarmas aplica true/false.
- Then MUST conservar polaridades y pines ajenos, sin contexto/casts/adaptadores.

### Requirement: SA4 — Evidencia y límites

B07-R1 MUST pasar host/sanitizadores, build/fixture ARM y Doxygen con USW existente.
MUST preservar Drivers y lógica B06/B12. MUST NOT inicializar/integrar main o añadir
GPIO/polaridad/fallos eléctricos al servicio, alarmas nuevas, timeout o regulación.
Antes: acreditaba aceptación declarada por proveedores; ahora acredita sólo despacho.

#### Scenario: S8 — Verificación
- Given contrato y consumidores migrados.
- When se ejecutan controles y se revisa diff.
- Then MUST registrar evidencia por escenario separando host, build ARM y placa pendiente.

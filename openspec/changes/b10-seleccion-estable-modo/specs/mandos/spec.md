# Mandos — candidato B10

Origen R05/R06/R10/R11/R15; alcance acordado el 2026-11-09.
Delta sobre B04; resultado pendiente de aceptación, sin promover contrato vigente.

## MODIFIED Requirements

### Requirement: MA3 — Aislamiento y compatibilidad

Mandos MUST ser C11 sin heap, flotantes, globals mutables o hardware. Instancias
MUST ser independientes y serializadas externamente. Sólo VALIDO habilita consumo;
MUST conservar B04/B01 y evitar Servicios→Aplicacion. B10 MAY filtrar un selector
lógico con muestras/tiempo externos. MUST NOT implementar ADC/DMA, GPIO/selector
físico, reloj/ISR real, filtrado analógico, calibración, PWM, regulación o main.
(Antes: B04 excluía selector y antirrebote del alcance original.)

#### Scenario: S6 — Instancias y consumidor B01
- Given mandos manuales B04 y selector lógico B10 independientes.
- When se actualizan/reinician y se adaptan salidas disponibles en pruebas.
- Then MUST preservar normalización y contratos B01/B08 sin hardware concreto.

## ADDED Requirements

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

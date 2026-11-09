# Alarma de temperatura — contrato candidato B06

Origen R07/R10/R11/R15; alcance aceptado por usuario 2026-11-09, resultado pendiente.
Umbral como parámetro de configuración, sin valor fijo acordado.

## Requirements

### Requirement: AT1 — Decisión con umbral configurable

Procesamiento MUST evaluar una lectura genérica Temperatura_Lectura con umbral
int32_t en centésimas Celsius explícito por llamada. Con estado VALIDA MUST devolver
ACTIVA sólo si temperatura>umbral; igual/inferior MUST devolver INACTIVA.
MUST admitir todo int32_t sin overflow, flotantes, redondeo o valor de umbral implícito.

#### Scenario: S1 — Fronteras configurables
- Given varios umbrales, incluidos negativos y cero, con lecturas válidas cercanas.
- When temperatura es menor, igual o mayor que cada umbral.
- Then MUST resultar INACTIVA, INACTIVA y ACTIVA respectivamente.

#### Scenario: S2 — Extremos y cambio de configuración
- Given INT32_MIN/MAX y una lectura constante con distintos umbrales.
- When se evalúan todas las relaciones y se cambia el parámetro por llamada.
- Then MUST comparar exactamente sin resta peligrosa ni recordar configuración anterior.

### Requirement: AT2 — Disponibilidad y errores explícitos

SIN_DATOS MUST producir ALARMA_SIN_DATOS; ERROR_FUENTE, ALARMA_ERROR_FUENTE;
ERROR_ARGUMENTO, ALARMA_ERROR_ARGUMENTO. NULL o enum desconocido MUST producir
ALARMA_ERROR_ARGUMENTO. La validez MUST decidirse antes de usar el campo numérico.
Esos resultados MUST ser distintos de ACTIVA/INACTIVA; cero válido no equivale a ausencia.
Cada llamada MUST devolver decisión completa, sin conservar una decisión anterior.

#### Scenario: S3 — Ausencia, errores y argumentos
- Given cero válido, ausencia, fallo de fuente, fallo de argumento, enum desconocido y NULL.
- When se evalúan, incluso con valor numérico extremo en una lectura no válida.
- Then MUST distinguir todos los estados y rechazar argumentos sin acceso inválido.

#### Scenario: S4 — Invalidación y recuperación
- Given secuencia válida activa→error→ausencia→válida inactiva→válida activa.
- When el llamador reemplaza su decisión con cada retorno.
- Then MUST obtener la secuencia correspondiente sin reutilizar temperatura/alarma anteriores.

### Requirement: AT3 — Separación y ausencia de efectos laterales

Evaluar MUST NOT modificar lectura, contexto/modos/consignas B01 ni dispositivos.
MUST consumir contrato del servicio Temperatura, sin conocer LM35, ADC, VREF o pines.
MUST ser C11, sin heap/globals mutables, reloj o esperas. Llamador MUST entregar
lectura legible/coherente y serializar acceso; const no garantiza atomicidad.

#### Scenario: S5 — Lecturas independientes e inmutables
- Given dos lecturas y umbrales distintos, una fallida.
- When se evalúan alternadamente y se comparan sus campos anteriores.
- Then MUST conservar ambos registros y no arrastrar estado entre evaluaciones.

#### Scenario: S6 — Integración con B05
- Given lecturas producidas por Inicializar/Actualizar de Temperatura, sin LM35.
- When se entregan a la aplicación con un umbral explícito.
- Then MUST evaluar Celsius/estado correctamente, incluidos recuperación y errores del servicio.

#### Scenario: S7 — Modos sin interferencia
- Given manual, automático esperando y automático con consigna activa.
- When se evalúan condiciones activa/ausente/fallida y continúa actualización B01.
- Then MUST mantener contexto y selección de mando, sin ordenar parada del motor.

### Requirement: AT4 — Verificación y alcance

B06 MUST pasar host/sanitizadores, regresiones, compilación/enlace ARM y Doxygen
de USW. MUST NOT integrar GPIO, LED/buzzer, histéresis, enclavamiento, timeout,
main o prioridad temporal. Resultado lógico no acredita señalización ni tiempos físicos.

#### Scenario: S8 — Evidencia reproducible
- Given código y contratos B06, con fuentes y controles previos preservados.
- When se ejecutan suites, fixture ARM, documentación y revisión del diff.
- Then MUST registrar resultados reales y mantener aceptación física pendiente.

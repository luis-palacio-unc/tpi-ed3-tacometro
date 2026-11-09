# Delta medición de velocidad — B03-R1 candidato

Origen R02/R15 y orden de revisión del usuario 2026-11-07. Sustituye solamente los
requisitos indicados del contrato vigente; archivo B03 aceptado inmutable.

## MODIFIED Requirements

### Requirement: MV1 — Conversión y unidades

El servicio MUST recibir uint32_t pulsos, duración_us y pulsos_por_vuelta positivo
de una instantánea/configuración coherente. MUST entregar uint32_t milésimas de
RPM: pulsos×60000000000/(duración_us×pulsos_por_vuelta), cercano/empate arriba,
sin redondeo intermedio ni recorte a 185 RPM. Error aritmético <=0,0005 RPM,
sin afirmar precisión física. Antes el factor 60 era fijo; ahora MUST ser explícito.
Configuración TPI MUST conservar 60 para un flanco/ranura, fuera del servicio.

#### Scenario: S1 — Referencias de TPI
- Given factor 60, duración 1000000 us y 0/60/185/200 pulsos.
- When se convierte.
- Then MUST resultar 0/60000/185000/200000 válidos.

#### Scenario: S2 — Equivalencias y redondeo
- Given factor 60 y ventanas 3/20000, 300/2000000, 1/3000000, 1/6000000, 1/16000000.
- When se convierten.
- Then MUST resultar 150000/150000/333/167/63 respectivamente.

#### Scenario: S8 — Configuraciones diferentes
- Given factores 1/12/60/100/1024/MAX con igual número de pulsos y ventana de 1 s.
- When se actualiza alternando factores por llamada.
- Then MUST resultar 60000, sin modificar/recompilar el servicio.

### Requirement: MV2 — Disponibilidad y errores observables

Inicializar MUST dejar SIN_DATOS/cero. Duración positiva y factor positivo sin
pulsos MUST ser cero válido. Prioridad MUST ser NULL, duración cero, factor cero,
resultado redondeado fuera de uint32_t. Factor cero MUST devolver
ERROR_PULSOS_POR_VUELTA; otros errores conservan códigos anteriores.
Todo error escribible MUST invalidar y borrar dato viejo; nueva ventana válida recupera.

#### Scenario: S3 — Ausencia y reinicio
- Given registro nuevo o lectura previa válida/errónea.
- When se inicializa.
- Then MUST quedar SIN_DATOS/cero, distinto de cero válido.

#### Scenario: S4 — Duración y recuperación
- Given lectura válida y después duración cero.
- When se actualiza y luego llegan 60 pulsos/1 s/factor 60.
- Then MUST informar ERROR_DURACION/cero y después 60000 válido.

#### Scenario: S5 — Representabilidad de TPI
- Given factor 60, pulsos MAX y duraciones 1/MAX/1000000000/999999999.
- When se convierte.
- Then MUST resultar error/1000000000/MAX/error sin overflow; cero pulsos es válido.

#### Scenario: S9 — Factor inválido y prioridad
- Given factor cero, datos previos, NULL y combinaciones de errores.
- When se llama y luego se entrega factor válido.
- Then MUST respetar prioridad, invalidar salida escribible y recuperar.

### Requirement: MV3 — API y aislamiento

La API MUST ser C11 sin heap, flotantes, periféricos, estado global mutable ni
dependencia de configuración física. NULL MUST devolver ERROR_ARGUMENTO sin acceso
(false al inicializar). Llamador MUST serializar captura/configuración/lectura y
detectar overflow previo. MUST NOT decidir ventana, filtrado, timeout, engranajes o main.

#### Scenario: S6 — Punteros e instancias
- Given NULL y dos lecturas con factores distintos.
- When se actualizan alternadamente y falla una.
- Then MUST rechazar NULL y conservar independiente la otra.

#### Scenario: S10 — Dominio numérico completo
- Given extremos uint32_t y ternas con numerador matemático superior a uint64_t.
- When se compara con oráculo entero exacto.
- Then MUST coincidir estado/valor, incluidos MAX/MAX/MAX →14 y valores diminutos →0.

### Requirement: MV4 — Evidencia de implementación

B03-R1 MUST pasar host C, oráculo exacto independiente, sanitizadores, regresiones,
enlace ARM y Doxygen ESW/USW. MUST distinguir ELF de ejecución física.

#### Scenario: S7 — Verificación reproducible
- Given servicio sin configuración/dispositivo y código/documentación nuevos.
- When se ejecutan CTest host/ARM, sanitizadores, consumidor aislado y Doxygen.
- Then MUST pasar y registrar evidencia; adquisición/tiempos físicos siguen pendientes.

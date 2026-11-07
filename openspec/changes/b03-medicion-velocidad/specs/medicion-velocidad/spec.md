# Medición de velocidad — contrato candidato B03

Origen: R02, soporte lógico de R04/R09; R14 limita consignas, no mediciones.
Alcance aprobado el 2026-11-06; resultado aún sin aceptación.

## Requirements

### Requirement: MV1 — Conversión y unidades

El servicio MUST convertir uint32_t pulsos y uint32_t duración en microsegundos
de una instantánea coherente, para 60 pulsos/vuelta del eje portadisco (un flanco
por ranura). MUST entregar uint32_t milésimas de RPM, sin limitar a 185 RPM.
MUST redondear al entero más cercano, empates hacia arriba; error aritmético
máximo 0,5 milésimas de RPM. Esto no garantiza precisión física del sensor/reloj.

#### Scenario: S1 — Referencias, cero y valores superiores a la consigna
- Given ventanas de 1 000 000 us con 0, 60, 185 y 200 pulsos.
- When se actualiza la lectura.
- Then MUST ser válida con 0, 60 000, 185 000 y 200 000 milésimas de RPM.

#### Scenario: S2 — Ventanas equivalentes y redondeo
- Given 3/20 000 y 300/2 000 000 pulsos/us; además 1/3 000 000,
  1/6 000 000 y 1/16 000 000.
- When se convierten las ventanas.
- Then MUST producir 150 000, 150 000, 333, 167 y 63 milésimas respectivamente.

### Requirement: MV2 — Disponibilidad y errores observables

Inicializar MUST producir SIN_DATOS y valor cero. Una ventana positiva sin pulsos
MUST ser válida, incluso si redondea a cero. Duración cero MUST producir
ERROR_DURACION. Resultado redondeado mayor que UINT32_MAX MUST producir
ERROR_RANGO. Todo error con salida escribible MUST reemplazar lectura anterior
con cero y su estado de error; MUST permitir recuperación con la siguiente ventana.

#### Scenario: S3 — Ausencia inicial y reinicio
- Given una lectura nueva o previamente válida/errónea.
- When se inicializa.
- Then MUST quedar SIN_DATOS y cero, distinto de cero válido.

#### Scenario: S4 — Duración inválida, valor viejo y recuperación
- Given una lectura válida y luego una ventana de duración cero.
- When se actualiza y después se entrega 60 pulsos/1 s.
- Then MUST informar ERROR_DURACION con cero y después 60 000 válidos.

#### Scenario: S5 — Extremos y representabilidad
- Given pulsos UINT32_MAX y duraciones 1, UINT32_MAX, 1 000 000 000
  y 999 999 999 us; también cero pulsos y 1 us.
- When se actualiza.
- Then MUST resultar error de rango, 1 000 000 000 válido, UINT32_MAX válido,
  error de rango y cero válido, sin wrap ni división por cero.

### Requirement: MV3 — API y aislamiento

La API MUST ser C11 sin heap, coma flotante, periféricos ni estado global mutable.
Salida NULL MUST devolver error sin acceso inválido (inicialización devuelve false).
El llamador MUST serializar acceso a cada lectura y captura coherente; MUST detectar
desbordes de adquisición antes de llamar. B03 MUST NOT decidir ventana física,
filtrado, timeout de frescura, control, relación de engranajes o integración del main.

#### Scenario: S6 — Punteros nulos e instancias
- Given salida NULL y dos lecturas independientes.
- When se inicializan/actualizan alternadamente.
- Then MUST rechazar NULL y mantener cada lectura independiente, sin arrastre de errores.

### Requirement: MV4 — Evidencia de implementación

B03 MUST pasar pruebas C host, sanitizadores, regresiones, enlace ARM completo
de su API y Doxygen estricto con IDs ESW/USW. MUST distinguir verificación lógica
y ELF de ejecución ARM, simulación o prueba física.

#### Scenario: S7 — Verificación reproducible
- Given código y documentación B03.
- When se ejecutan CTest host/ARM, sanitizadores y Doxygen.
- Then MUST pasar y registrar comandos/resultados; hardware y tiempos quedan pendientes.

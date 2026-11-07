# Temperatura — contrato candidato B05

Origen: R07/R09 y portabilidad R15; alcance aprobado el 2026-11-07.
Resultado pendiente de aceptación. Sólo conversión numérica del LM35 sin offset.

## Requirements

### Requirement: TE1 — Conversión y unidades

Temperatura MUST recibir muestra, escala_adc y referencia_uv uint32_t. Escala MUST
representar cuentas por referencia, con muestra<escala y escala>0; no el último código
inclusivo. Ejemplo: escala 4096 admite 0..4095. Referencia MUST ser positiva, en µV.
Con el modelo ideal LM35 10 mV/°C, MUST entregar uint32_t centésimas de °C:
muestra×referencia_uv/(escala_adc×100), redondeo cercano/empate arriba.
MUST evitar redondeos intermedios; error aritmético <=0,005 °C, sin garantizar
precisión del ADC, rango real del sensor o conexión física.

#### Scenario: S1 — Referencias y escala

- Given 125/1500 y 500/1500 muestras/escala con referencia 3 000 000 µV.
- When se convierten, además del último código admisible de otras escalas.
- Then MUST obtener 2500 y 10000 centésimas, respectivamente; muestra=escala es inválida.

#### Scenario: S2 — Configuraciones equivalentes

- Given configuraciones distintas que representan la misma tensión ideal del sensor.
- When se convierten sin fijar resolución o referencia de placa.
- Then MUST entregar igual temperatura; cambiar configuración por llamada está permitido.

#### Scenario: S3 — Redondeo único

- Given muestra 1, escala 4, referencias 4999, 5000 y 5001 µV.
- When se convierte sin cuantizar primero a µV/mV enteros.
- Then MUST producir 12, 13 y 13 centésimas; extremos de empate se resuelven hacia arriba.

### Requirement: TE2 — Disponibilidad y errores

Inicializar MUST dejar SIN_DATOS/cero. Actualizar MUST reemplazar valor/estado sin
leer datos anteriores. NULL MUST devolver ERROR_ARGUMENTO (false al inicializar),
sin acceso. Escala cero, referencia cero y muestra>=escala MUST producir
ERROR_ESCALA, ERROR_REFERENCIA y ERROR_MUESTRA, en esa prioridad después de NULL.
Errores con salida escribible MUST borrar valor e invalidarla; dato válido siguiente
MUST recuperar. Sólo VALIDA habilita consumo; cero ideal válido difiere de ausencia.

#### Scenario: S4 — Inicio y reinicio

- Given salida nueva, válida o errónea.
- When se inicializa y luego se convierte muestra cero con escala/referencia positivas.
- Then MUST pasar de SIN_DATOS/cero a VALIDA/cero, sin confundir disponibilidad y valor.

#### Scenario: S5 — Errores y recuperación

- Given lectura válida anterior, combinaciones inválidas y salida NULL.
- When se actualiza y luego llega una entrada válida.
- Then MUST respetar prioridad, invalidar el dato viejo y recuperar sin saturación.

### Requirement: TE3 — Límites y aislamiento

Todas las entradas uint32_t MUST tener resultado definido sin overflow ni división
por cero. El dominio válido cabe en uint32_t, hasta 42949673 centésimas; MUST NOT
imponer límites físicos de una variante no confirmada. VALIDA indica conversión
numérica, no sensor funcionando. Instancias MUST ser independientes, sin heap,
flotantes, globals mutables ni periféricos. Llamador MUST validar adquisición,
captura coherente y acceso serializado; no hay timeout de frescura.

#### Scenario: S6 — Extremos de tipos

- Given muestra=UINT32_MAX-1, escala=referencia=UINT32_MAX y otras fronteras.
- When se convierte sin sumar un término que desborde el numerador.
- Then MUST producir 42949673; casos inválidos se rechazan antes de dividir.

#### Scenario: S7 — Independencia

- Given dos salidas con configuraciones diferentes.
- When una se actualiza/reinicia/falla.
- Then MUST conservar la otra y no depender de Aplicacion ni dispositivos.

### Requirement: TE4 — Verificación y alcance

B05 MUST pasar C host/sanitizadores, enlace ARM, regresiones B00–B04 y Doxygen ESW/USW.
MUST NOT implementar ADC/DMA, offset/calibración, negativos, alarma, UART o main.

#### Scenario: S8 — Evidencia

- Given código y documentación B05.
- When se verifican escenarios, build, documentación, recursos y diff.
- Then MUST registrar ejecuciones reales y distinguir host/ELF de simulación/hardware pendientes.

# Temperatura — contrato vigente

Origen R07/R09/R15; B05 corregido aceptado por el usuario el 2026-11-07.
Alcance lógico; adquisición y aceptación física pendientes.

## Requirements

### Requirement: TE1 — Conversión específica en dispositivo

LM35 MUST convertir muestra/escala_adc/referencia_uv uint32_t a uint32_t centésimas
Celsius, modelo nominal 10 mV/°C sin offset. Escala son cuentas por referencia;
0<=muestra<escala. MUST redondear una vez al cercano/empate arriba, error aritmético
<=0,005 °C, sin imponer precisión/rango eléctrico no confirmado.
Servicios/Temperatura MUST NOT conocer esta curva, códigos ADC o parámetros eléctricos.

#### Scenario: S1 — Referencias

- Given muestras 125/1500 y 500/1500 con 3000000 µV.
- When LM35 convierte y se adapta a la fuente genérica.
- Then MUST producir 2500/10000 centésimas, sin fórmula LM35 en el servicio.

#### Scenario: S2 — Equivalencia y escala

- Given configuraciones distintas con igual tensión ideal y último código admisible.
- When LM35 convierte.
- Then MUST conservar temperatura; muestra=escala se rechaza.

#### Scenario: S3 — Redondeo y límites

- Given muestra 1/4 con 4999/5000/5001 µV y MAX−1/MAX/MAX.
- When LM35 convierte sin doble redondeo ni sumar al numerador.
- Then MUST devolver 12/13/13 y 42949673, sin overflow.

### Requirement: TE2 — Servicio genérico en unidades físicas

Temperatura MUST recibir int32_t centésimas Celsius y estado de fuente VALIDA,
SIN_DATOS o ERROR. MUST conservar exactamente datos válidos, sin recalibrar ni limitar
al rango de un sensor. SIN_DATOS MUST producir SIN_DATOS/cero; ERROR, ERROR_FUENTE/cero.
Inicializar MUST dejar SIN_DATOS/cero. NULL MUST devolver ERROR_ARGUMENTO (false al
inicializar); estado de fuente desconocido MUST invalidar con ERROR_ARGUMENTO/cero.
Una fuente válida posterior MUST recuperar. Sólo VALIDA habilita consumo.

#### Scenario: S4 — Magnitudes y disponibilidad

- Given fuentes genéricas con 2534, cero, -725 e INT32_MIN/MAX centésimas.
- When llegan como válidas, ausentes o fallidas.
- Then MUST preservar sólo valores válidos, diferenciando cero válido de ausencia/error.

#### Scenario: S5 — Invalidación y recuperación

- Given lectura previa válida, NULL, estado desconocido y ausencia/error de fuente.
- When se actualiza y luego llega nueva lectura válida.
- Then MUST aplicar errores definidos, borrar datos viejos y recuperar sin depender del sensor.

### Requirement: TE3 — Aislamiento y errores de dispositivo

LM35 MUST informar NULL/escala cero/referencia cero/muestra excesiva en ese orden,
borrar salida escribible ante error y recuperar; inicializar lectura deja SIN_DATOS/cero.
Su dominio válido cabe en uint32_t. Temperatura MUST compilar/enlazar sin dispositivo
concreto y sin depender de Aplicacion. Dispositivos MUST NOT depender del servicio.
Adaptación MUST mapear errores concretos a ERROR de fuente; no aplicar conversiones
en aplicación. Instancias MUST ser independientes, acceso serializado externo.

#### Scenario: S6 — Argumentos y fronteras

- Given errores combinados y extremos uint32_t del dispositivo/int32_t del servicio.
- When se ejecutan APIs y adaptación.
- Then MUST preservar prioridades y tipos, sin división inválida, overflow ni valor viejo válido.

#### Scenario: S7 — Otra fuente y estados independientes

- Given un consumidor de Temperatura sin LM35, dos instancias y adaptación LM35 separada.
- When se alternan fuentes o falla una.
- Then MUST funcionar con Celsius/estado genéricos y conservar la instancia ajena.

### Requirement: TE4 — Evidencia y alcance

B05 MUST pasar host/sanitizadores, ARM, regresiones B00–B04 y Doxygen ESW/USW.
MUST NOT adquirir ADC/DMA ni implementar acondicionamiento negativo LM35, alarma o main.

#### Scenario: S8 — Verificación

- Given código corregido y reproducción de acoplamiento previo.
- When se ejecutan suites, enlaces separados, documentación y revisión.
- Then MUST registrar evidencia real; integración/tiempos eléctricos permanecen pendientes.

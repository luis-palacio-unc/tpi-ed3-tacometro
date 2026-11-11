# Regulación de velocidad — contrato vigente

Origen R04/R06/R10/R11/R14/R15, ESW.2.1.2. B17 aceptado por el usuario el 2026-11-11,
sobre implementación 8f1793f36be1c73e63d8e8f0bbc9ca26852d244a. Reauditoría Claude r2: APROBAR, sin hallazgos nuevos.
Alcance lógico; cadencia real, integración y funcionamiento físico pendientes.
[Diseño aceptado](../../changes/archive/2026-11-11-b17-regulador-velocidad/design.md)
y [evidencia de aceptación](../../changes/archive/2026-11-11-b17-regulador-velocidad/verify-report.md).

## Requirements

### Requirement: RV1 — PI discreto y unidades

MUST exponer inicializar/regular/reiniciar aparte de B01. Un paso representa 500 ms.
Entradas: consigna uint32_t RPM y lectura B03 uint32_t milésimas, sin recortarla a 185.
PI MUST usar ganancias num/den positivas discretizadas para 500 ms, mando uint16_t 0..1000;
sin float/reloj.

#### Scenario: S1 — Paso y saturación

- Given instancia nueva con defaults; otra con Kp=Ki=1/1000, lectura válida500milésimas.
- When primera regula100RPM dos veces con medición0; segunda regula1RPM.
- Then MUST dar400/500 (I100/200), y1 en empate0,5; siempre0..1000.

### Requirement: RV2 — Redondeo y anti-windup

Salida MUST usar integral previa y redondear al cercano/empate arriba tras limitar
PI a 0..1000. Sólo con dato válido MUST integrar después, limitando integral a 0..1000,
excepto si PI sin limitar≥1000/error positivo o≤0/error negativo: MUST congelar.

#### Scenario: S2 — Desaturación

- Given 185 RPM/medición 0 válida, dos pasos previos y 40 pasos saturados.
- When consigna baja a 100 con igual medición.
- Then MUST devolver 770 en 1 paso; quitar sólo anti-windup MUST hacer fallar la prueba.

### Requirement: RV3 — Retención y reinicio

Estado distinto de VALIDA MUST ignorar valor recibido y reutilizar última válida
hasta N fallos consecutivos, sin integrar. Válida nueva MUST reemplazar historia/resetear
contador. Sin historia o en N+1 MUST emitir 0/reiniciar integral, historia y contador.
Reiniciar MUST preservar configuración y descartar historia; instancias independientes.

#### Scenario: S3 — Límites y recuperación

- Given N=4 e historia válida; estados SIN_DATOS, errores o desconocidos.
- When se recorren rachas hasta N/N+1 y se intercala una válida.
- Then MUST congelar I hasta N; N+1 borra/da0; válida antes preserva I, después empieza en0.

#### Scenario: S4 — Sin historia y reinicio

- Given arranque, reinicio, o N=1/65535.
- When llegan inválidas y se cruza el límite.
- Then MUST respetar N/N+1 sin wrap; sin historia siempre 0; otra instancia intacta.

### Requirement: RV4 — Errores y configuración

Prioridad MUST ser NULL→configuración→consigna>185→consigna 0→medición.
Errores propios MUST dar mando escribible 0/reinicio dinámico. Consigna 0 MUST devolver PARADO/0
y reiniciar sin usar medición. Control.h MUST proteger componentes1..1000/N1..65535 con #ifndef/default/#error;
runtime MUST validar iguales cotas. TPI_CONTROL_PERIODO_MS=500 MUST ser constante
contractual: cambiarla exige recalcular ganancias/revisar contrato. Defaults
provisionales; configuración inválida MUST deshabilitar instancia.

#### Scenario: S5 — Prioridades

- Given NULL, init fallida, consigna 186/MAX/0 y medición no válida combinados.
- When se inicializa/regula/reinicia.
- Then MUST respetar prioridad, borrar mando previo y recuperar tras init válida.

#### Scenario: S6 — Representabilidad

- Given medición 0/185000/185001/UINT32_MAX y ganancias extremas admitidas.
- When regula y compila probes inválidos.
- Then MUST evitar UB, conservar dominio medido y rechazar configuración por diagnóstico específico.

### Requirement: RV5 — Composición y desempeño simulado

Fixture MUST componer B01+PI+Driver_Motor; CERO/MANUAL reinician PI; sólo AUTOMATICA
regula. MUST preservar espera/nueva consigna B01, sin enviar RPM como PWM.
Modelo entero fijado MUST lograr sobrepico≤10% del escalón y establecimiento
≤20 pasos/±2 RPM, confirmado; MUST NOT presentarse como sintonía física.

#### Scenario: S7 — Transiciones

- Given manual→auto→consigna→manual→reentrada y proveedor fallido.
- When fixture selecciona/emite.
- Then MUST enviar un mando/cero por selección, reiniciar historia y observar retorno del servicio.

#### Scenario: S8 — Escalones

- Given modelo/ganancias/horizonte de verify-report, escalones 0→100→185.
- When simula 200 pasos.
- Then MUST cumplir métricas fijadas por escalón, sin reajustarlas después.

### Requirement: RV6 — Evidencia

MUST pasar Fraction, host/san, ARM sin float, Doxygen USW desde .8 y conservar
Drivers/servicios/B01/B06/specs aceptadas. Mutar redondeo externamente MUST hacer
fallar S1 u oráculo.

#### Scenario: S9 — Oráculo

- Given secuencias deterministas/aleatorias y configuración de verify-report.
- When compara cada paso C/Fraction.
- Then MUST coincidir estado/mando/integral/historia/contador exactamente.

#### Scenario: S10 — Regresiones y modalidades

- Given implementación futura y 29 suites host/13 controles ARM previos.
- When ejecuta verificaciones nuevas y anteriores.
- Then MUST pasar sin omisiones, registrar recursos/diff y dejar hardware pendiente.

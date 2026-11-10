# Base temporal TC y wrapper — candidato B11

Origen R01/R02/R08/R09/R10/R11/R15; usuario 2026-11-10. Candidato sin aceptar.

## Requirements

### Requirement: TC1 — Configuración de cuenta

MUST usar TIMER0 exclusivo continuo, sin ISR periódica/acumulador software/reinicio
por consulta o match. MUST recibir PCLK real estable uint32_t 1..120000000 Hz y
período uint32_t 1..INT32_MAX ms. Default TPI MUST ser 1 ms configurable.
N=PCLK*período/1000 MUST ser entero exacto y 1..2^32; PR=N-1. Prioridad de errores
MUST ser reloj, período y prescaler. Error MUST deshabilitar/reiniciar timer y
dejar salida cero/inválida; reinicio válido MUST recuperar desde cero.
MUST NOT modificar PLL/PCLKSEL/flash/pines ni otros recursos periféricos.

#### Scenario: S1 — Prescaler y base
- Given bases1/2/5ms y extremos N=1/2^32.
- When se inicializa.
- Then MUST configurar cuenta exacta y ACTIVA/cero, sin IRQ ni reset por match.

#### Scenario: S2 — Configuración inválida
- Given reloj/período inválido o N inexacto/excesivo.
- When se inicializa y recupera.
- Then MUST respetar prioridad, deshabilitar y recuperar.

### Requirement: TC2 — Lectura y ciclo de vida

Antes de inicializar MUST publicar SIN_DATOS/cero sin acceder TC. Leer MUST consultar
TC una vez y producir uint32_t ms modular = TC*período, sin parar/reiniciar ni contar
en software. ACTIVA habilita tiempo actual; DETENIDA MUST entregar tiempo congelado.
Detener MUST pasar ACTIVA→DETENIDA; otros estados MUST conservarse. Reiniciar MUST
abrir nueva época desde cero. NULL MUST producir ERROR_ARGUMENTO sin efectos propios.
Acceso MUST ser desde lazo privilegiado serializado y timer exclusivo.

#### Scenario: S3 — Cuenta y consultas
- Given timer activo sin consultas/ISR y dos consumidores.
- When consultan tiempo/intervalos.
- Then MUST reflejar TC real normalizado sin modificar registros ni marcas ajenas.

#### Scenario: S4 — Ausencia/parada/reinicio
- Given cualquier estado y destinos NULL.
- When se lee/detiene/reinicia.
- Then MUST distinguir cero/ausencia/error, congelar y reiniciar época.

### Requirement: TC3 — Wrapper independiente del MCU

Tiempo MUST usar proveedor bool(void*,uint32_t*) reemplazable, sin incluir/enlazar
dispositivos/CMSIS/configuración física. Leer y Transcurrido MUST validar destino,
fuente y función; argumentos inválidos MUST devolver ERROR_ARGUMENTO sin proveedor.
Interfaz válida MUST invocar una vez con contexto exacto; false MUST producir
NO_DISPONIBLE/cero ignorando valor escrito; true MUST producir VALIDO incluso cero.
MUST limpiar salidas escribibles ante error, sin heap/espera/retención/global mutable.

#### Scenario: S5 — Proveedores y fallos
- Given proveedores independientes, NULL/interfaz incompleta o fallo con salida sucia.
- When se consulta y luego recupera la fuente.
- Then MUST preservar diagnóstico, contexto, llamada única y recuperación sin MCU.

### Requirement: TC4 — Unidades y rollover

Normalización MUST conservar ms al cambiar resolución, con producto modular deliberado.
Transcurrido MUST restar marcas uint32_t modularmente y rechazar delta>INT32_MAX
con ERROR_INTERVALO/cero; prioridad MUST ser fuente antes de intervalo. Misma época
y duración real <2^31 ms MUST ser precondiciones; reinicio exige marcas nuevas.
MUST NOT prometer detectar vueltas completas/épocas mezcladas o precisión física.

#### Scenario: S6 — Fronteras temporales
- Given bases 1/2/3/5 ms y rollover TC/ms, marcas iguales o deltas de semirrango.
- When se lee/calcula transcurrido.
- Then MUST conservar unidades/modularidad, admitir INT32_MAX y rechazar semirrango/excesos.

### Requirement: TC5 — Composición y evidencia

MUST preservar B00–B10/B03-R1, main/startup/linker y archivos aceptados.
MUST reutilizar el driver TIM existente para configurar/leer/detener; MAY ampliarlo
de forma compatible para cuenta continua sin cambiar PCLK. MUST usar un único
módulo dispositivo Timer (Init/Read/Stop/Source), sin puerto intermedio.
Sólo pruebas MAY componer proveedor→Tiempo→Mandos; MUST documentar USW y ejecutar
host/sanitizadores/regresiones, ARM Debug/Release, inspección de recursos y Doxygen
estricto. TC/IRQ/configuración MCU MUST quedar encapsulados por dispositivos y sus drivers,
fuera del servicio.

#### Scenario: S7 — Integración
- Given proveedor real modelado y selector B10 con intervalo en ms.
- When cambia base, consulta en frontera o fuente se detiene/falla.
- Then MUST respetar B10 y consumir sólo tiempo válido, sin integrar main.

#### Scenario: S8 — Evidencia por modalidad
- Given hardware modelado/encabezados reales ARM y unidades documentadas.
- When se verifica configuración/lectura, build, aislamiento, regresiones y recursos.
- Then MUST registrar resultados reales, distinguiendo host/ARM de simulación MCU/placa pendientes.

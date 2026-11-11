# UART dispositivo — candidato B13

Origen R09/R10/R11/R15. Alcance/UART0 acordados2026-11-10; resultado sin aceptar.

## Requirements

### Requirement: UD1 — Interfaz y argumentos

UART MUST exponer uint32_t UART_Read(uint8_t *destino,uint32_t capacidad) y
uint32_t UART_Write(const uint8_t *datos,uint32_t longitud). Tamaños/retornos son
bytes, no caracteres/tramas. MUST operar sobre UART0 mediante biblioteca original
en modo no bloqueante. NULL o tamaño cero MUST retornar0 sin tocar driver/buffer.
Punteros no NULL MUST designar almacenamiento válido del tamaño indicado y estable
durante la llamada; acceso serializado. MUST NOT conservar punteros/contextos.

#### Scenario: S1 — Argumentos vacíos o inválidos
- Given NULL con tamaño cero/no cero o buffer con tamaño cero.
- When se lee/escribe.
- Then MUST retornar0 sin consumir/emitir bytes ni modificar memoria; una llamada válida posterior funciona.

### Requirement: UD2 — Lectura de bytes disponibles

UART_Read MUST delegar una vez una petición válida y devolver el número recibido
0..capacidad. MUST entregar bytes en orden, incluyendo0x00/0xFF, sin NUL añadido.
MUST modificar sólo el prefijo recibido y preservar resto/canarios. MUST NOT
esperar a completar capacidad ni inventar dato si no hay recepción.

#### Scenario: S2 — Recepción completa o parcial
- Given buffer con capacidad y fuente con cantidad distinta de bytes disponibles.
- When se lee.
- Then MUST devolver/copiar sólo la cantidad recibida, con orden y límites correctos.

#### Scenario: S3 — Sin bytes disponibles
- Given destino inicializado y recepción vacía.
- When se lee.
- Then MUST devolver0 sin alterar el destino ni esperar nuevos datos.

### Requirement: UD3 — Escritura y progreso parcial

UART_Write MUST delegar una vez una petición válida y devolver el número que el
driver entrega al transmisor,0..longitud. MUST preservar origen y orden de bytes;
MUST NOT añadir terminador/reintentar el resto/esperar fin físico. El consumidor
MUST gestionar resto/offset fuera del dispositivo; retorno no acredita recepción PC
ni errores eléctricos/de línea. Configuración previa MUST habilitar FIFO/transmisión.

#### Scenario: S4 — Transmisión completa, parcial o no disponible
- Given origen binario y capacidad disponible variable, incluida cero.
- When se escribe.
- Then MUST retornar progreso real y transmitir sólo ese prefijo, sin modificar origen.

#### Scenario: S5 — Continuación por el consumidor
- Given una escritura parcial y disponibilidad posterior.
- When el consumidor llama con el sufijo pendiente.
- Then MUST completar bytes en orden sin pérdida/duplicación ni estado retenido en UART.

### Requirement: UD4 — Capas, configuración y evidencia

Dispositivo MUST reutilizar driver original sin duplicar operaciones FIFO/registros.
MUST NOT modificar Drivers/servicios/main ni implementar protocolos, init/PINSEL/
reloj, colas propias, ISR/DMA, heap o globals mutables. Configuración UART/pines/
reloj/FIFO/DLAB=0 y exclusividad con ISR/DMA MUST ser precondiciones externas.
MUST documentar ESW/USW, probar límites/bytes/progreso en host y compilar ARM.
MUST separar prueba con doble de API de funcionamiento físico/tiempos reales.

Asignación MUST ser UART0, TX P0.2/RX P0.3, PINSEL0 función01 según tabla80
del manual. UART_BAUDRATE MUST ser parámetro entero positivo uint32_t configurable
al compilar en UART.h, default115200. Cero/negativo/exceso MUST rechazarse al compilar;
esta validación MUST NOT afirmar tasa físicamente realizable. Parámetro/pines se
aplicarán al configurar periféricos en integración final, no mediante init nuevo aquí.

#### Scenario: S7 — Pines y parámetro configurables
- Given manual y compilaciones con default/overrides positivos o inválidos.
- When se documenta asignación y se compila el header/módulo.
- Then MUST conservar TX P0.2/RX P0.3, aceptar overrides representables y rechazar inválidos, sin iniciar hardware.

#### Scenario: S6 — Verificación y alcance
- Given funciones con driver emulado y headers originales para compilar módulos.
- When se ejecutan host/sanitizadores/build/Doxygen y revisión de diff/hashes.
- Then MUST preservar biblioteca/servicios y registrar hardware/integración pendientes.

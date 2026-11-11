# Delta B18/I1 — uart-dispositivo

Candidato de la vía rápida B18 (observación O6 aceptada por el usuario). Sólo
sincronizar al cerrar la integración.

## MODIFIED Requirements

### Requirement: UD1 — Interfaz y argumentos
UART MUST exponer uint32_t UART_Read(uint8_t *destino,uint32_t capacidad),
uint32_t UART_Write(const uint8_t *datos,uint32_t longitud) y void UART0_Init(void).
Tamaños/retornos son bytes. Read/Write MUST ser no bloqueantes y operar sobre
buffers circulares propios de UART0. NULL o tamaño cero MUST retornar 0 sin tocar
biblioteca, buffers ni NVIC. MUST NOT conservar punteros del llamador. Read y Write
MUST llamarse sólo desde el lazo principal. (Antes: delegación directa en el driver,
sin init ni estado.)

#### Scenario: S1 — Argumentos vacíos o inválidos
- Given NULL con tamaño cero/no cero o buffer con tamaño cero.
- When se lee/escribe.
- Then MUST retornar 0 sin consumir/encolar bytes ni pedir interrupción; una llamada
  válida posterior funciona.

### Requirement: UD2 — Lectura desde el buffer de recepción
UART_Read MUST retirar del buffer circular de recepción (64 bytes) hasta `capacidad`
bytes en orden de llegada, incluyendo 0x00/0xFF, sin NUL añadido, y devolver la
cantidad. MUST modificar sólo el prefijo copiado. MUST NOT acceder al periférico,
esperar ni inventar datos. (Antes: una llamada a UART_Receive NONE_BLOCKING.)

#### Scenario: S2 — Recepción completa o parcial
- Given bytes recibidos por el ISR y capacidad menor, igual o mayor.
- When se lee una o más veces.
- Then MUST devolver/copiar sólo lo disponible, con orden y límites correctos, y
  continuar sin pérdida ni duplicación, también cuando los índices dan la vuelta.

#### Scenario: S3 — Sin bytes disponibles
- Given destino inicializado y buffer vacío.
- When se lee.
- Then MUST devolver 0 sin alterar el destino.

### Requirement: UD3 — Escritura encolada y progreso parcial
UART_Write MUST copiar al buffer circular de transmisión (128 bytes) los bytes que
entren, devolver esa cantidad (0..longitud) y pedir la interrupción de UART0 para
que el ISR arranque el envío si el transmisor está ocioso. MUST preservar origen y
orden; MUST NOT escribir U0THR, añadir terminador, reintentar ni esperar. Con el
buffer vacío una trama de 53 bytes MUST encolarse completa y salir completa. El
consumidor MUST gestionar resto/offset; el retorno no acredita recepción en la PC.
(Antes: una llamada a UART_Send NONE_BLOCKING, limitada a la FIFO de 16.)

#### Scenario: S4 — Transmisión completa, parcial o no disponible
- Given 100 bytes encolados sin atender interrupciones.
- When se escriben 53 y luego 1.
- Then MUST devolver 28 y 0, sin modificar el origen ni perder los ya encolados.

#### Scenario: S5 — Continuación por el consumidor
- Given una escritura parcial y espacio liberado después.
- When el consumidor llama con el sufijo pendiente.
- Then MUST completar los bytes en orden sin pérdida/duplicación.

#### Scenario: S8 — Trama de 53 bytes
- Given buffer vacío y transmisor ocioso.
- When se escriben 53 bytes y se atienden las interrupciones.
- Then la línea MUST recibir los 53 bytes en orden, en cargas de 16, 16, 16 y 5.

### Requirement: UD4 — Capas, configuración y evidencia
Dispositivo MUST usar la biblioteca original para configurar y para cada acceso a
RBR/THR/IIR/LSR/IER, sin acceso directo a registros. MUST NOT modificar Drivers ni
implementar protocolo, DMA o heap. Asignación MUST ser UART0, TX P0.2/RX P0.3,
PINSEL0 función 01 según tabla 80 del manual. UART_BAUDRATE MUST ser parámetro
entero positivo uint32_t configurable al compilar en UART.h, default 9600; cero/
negativo/exceso MUST rechazarse al compilar. MUST documentar ESW/USW, probar en host
con dobles de la biblioteca y compilar ARM, separando esa evidencia del
funcionamiento físico. (Antes: default 115200, sin init/colas/ISR.)

#### Scenario: S7 — Pines y parámetro configurables
- Given manual y compilaciones con default/overrides.
- When se inicializa con el default.
- Then MUST pedir 9600 baudios y conservar TX P0.2/RX P0.3 función 01.

## ADDED Requirements

### Requirement: UD5 — Inicialización
UART0_Init MUST, en orden: deshabilitar UART0_IRQn, vaciar los buffers, UART_Init con
UART_BAUDRATE 8N1, UART_FIFOConfig con FIFO habilitada/vaciada y disparo de 1
carácter, configurar P0.2 (sin resistencias) y P0.3 (pull-up) en función 01,
UART_TxEnable, habilitar interrupciones RBR y THRE, fijar prioridad 2 y habilitar
UART0_IRQn. El nombre UART0_Init MUST evitar la colisión con UART_Init de la
biblioteca.

#### Scenario: S9 — Parámetros acordados
- Given dobles de la biblioteca y del NVIC.
- When se llama UART0_Init.
- Then MUST observarse esa secuencia y esos parámetros, sin otra IRQ habilitada.

### Requirement: UD6 — ISR de UART0
UART0_IRQHandler MUST leer U0IIR una vez; con causa RDA o CTI MUST pasar al buffer
de recepción los bytes disponibles, como máximo 16 por entrada; si el buffer está
lleno MUST leer y descartar el byte nuevo. Después, con o sin causa, MUST cargar
hasta 16 bytes pendientes en la FIFO de transmisión sólo si U0LSR.THRE = 1. MUST ser
el único que escribe U0THR. MUST NOT esperar ni validar errores de línea.

#### Scenario: S10 — Cebado y reposo
- Given transmisor ocioso tras vaciar el buffer, y luego FIFO todavía ocupada.
- When se escribe en cada situación.
- Then en la primera MUST transmitir sin esperar otra causa del periférico; en la
  segunda MUST NOT escribir U0THR hasta que THRE vuelva a 1.

#### Scenario: S11 — Buffer de recepción lleno
- Given 80 bytes recibidos sin lecturas.
- When se atienden las interrupciones y luego se lee.
- Then MUST conservar los primeros 64, descartar 16, dejar vacía la FIFO del
  periférico y aceptar bytes posteriores.

# TPI EDIII — Tacómetro digital

Firmware C para LPC1769/OM13085, CLion y SDD por bloques.
Leer primero [ESTADO](docs/agente/ESTADO.md). Ver también
[CONTEXTO](docs/agente/CONTEXTO.md), [ORQUESTADOR](docs/agente/ORQUESTADOR.md),
[MAPA_REQUISITOS](docs/agente/MAPA_REQUISITOS.md) y
[verificación B00](openspec/changes/archive/2026-11-06-b00-build-lpc1769/verify-report.md).

## Compilar en CLion o terminal

Desde la raíz del proyecto, usar el preset CMake `lpc1769`:

```sh
cmake --preset lpc1769
cmake --build --preset lpc1769
ctest --preset lpc1769
```

Requisitos ya disponibles en el entorno de diagnóstico: CMake >=3.20, GCC ARM,
GNU binutils ARM, Python >=3.8 estándar y compilador C host para la prueba de objetos
host. No se incorporan paquetes Python, libc ni frameworks externos.
Si las herramientas ARM no están en PATH, indicar el directorio bin con
`-DARM_TOOLCHAIN_DIR=/ruta/al/bin` al configurar. Un directorio inválido falla.

Salidas: `build/lpc1769/firmware.elf`, `firmware.map` y `memoria.txt`.
El build ejecuta automáticamente el verificador del ELF; CTest agrega casos negativos.
`firmware.elf` es para el MCU: no se ejecuta como programa de escritorio.
No está configurada todavía una sesión de depuración/flasheo de CLion.

Perfil host: compila objetos y ejecuta pruebas C de Procesamiento, Comunicacion, Medicion_Velocidad, Mandos, Temperatura, LM35, Driver_Alarmas, Driver_Motor, incluidas sus pruebas de integración, sin ejecutar main del firmware:

```sh
cmake --preset host-objects
cmake --build --preset host-objects
ctest --preset host-objects
```

Para mantener todos los resultados fuera del proyecto también se admite:

```sh
cmake -S . -B /tmp/ed3-arm -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"
cmake --build /tmp/ed3-arm
ctest --test-dir /tmp/ed3-arm --output-on-failure
```

## Alcance actual

B00 implementa build ARM, vectores, linker y main en bucle. El startup copia .data,
borra .bss y llama main; las IRQ sin implementación quedan en un handler por defecto.
No configura reloj/PLL ni periféricos, ni llama SystemInit heredado. No supone una
frecuencia de reloj al entrar desde el bootloader. Pila reservada de 2 kB, sin heap;
las dos SRAM AHB quedan sin asignar. La imagen reserva CRP con valor 0xffffffff.

La verificación estática y las pruebas de herramientas no acreditan arranque físico,
tiempos ni funcionamiento eléctrico. No se ejecutaron firmware ni pruebas heredadas
en placa. La revisión física de PCB/silicio permanece pendiente.

Se preservan `Aplicacion/`, `Servicios/`, `Dispositivos/` y `Drivers/`.
La biblioteca de Drivers no se compila completa en B00; se verifican sus encabezados
CMSIS. Los demás módulos del tacómetro siguen pendientes de implementación.

Repositorio público: https://github.com/luis-palacio-unc/tpi-ed3-tacometro
B00 aceptado y archivado. Publicación inicial en main autorizada por el usuario;
no extiende autorización a bloques futuros. [Contrato build vigente](openspec/specs/build/spec.md).

B01 aceptado/archivado con límite 185 RPM. [Contrato de modos vigente](openspec/specs/modos/spec.md).
B02 aceptado y archivado. [Contrato de comunicación vigente](openspec/specs/comunicacion/spec.md).
B03 aceptado y archivado. [Contrato de medición vigente](openspec/specs/medicion-velocidad/spec.md).
B04 aceptado y archivado. [Contrato de mandos vigente](openspec/specs/mandos/spec.md).
B05 corregido y B05-R1 aceptados y archivados. [Contrato de temperatura vigente](openspec/specs/temperatura/spec.md)
y [evidencia](openspec/changes/archive/2026-11-07-b05-conversion-temperatura/verify-report.md).

## B01 y documentación incremental

Procesamiento.h/.c implementan selección de modo/fuente, límite 0..185 RPM y validez temporal de consignas,
sin hardware ni regulación PI/PID. main sigue mínimo. Contratos, escenarios y evidencia
están en openspec/changes/archive/2026-11-06-b01-gestion-modos/.

Para instrumentar pruebas host (en un build independiente):

```sh
cmake -S . -B /tmp/ed3-host-san -DTPI_BUILD_ARM=OFF -DTPI_HOST_SANITIZERS=ON
cmake --build /tmp/ed3-host-san
ctest --test-dir /tmp/ed3-host-san --output-on-failure
```

Doxygen ya disponible en este entorno; no es requisito para compilar el firmware.
Desde la raíz, `doxygen docs/doxygen/Doxyfile` genera HTML/XML parciales en
build/doxygen/. Ver [convención documental](docs/doxygen/README.md). Estos resultados
no constituyen aún la documentación final de todos los módulos.

## B02: recepción lógica de consignas

Enviar ASCII `RPM 185` terminado en LF o CRLF; se admiten 1..3 dígitos, ceros
iniciales y valores 0..185. Las tramas inválidas se rechazan, sin saturar valores.
Comunicacion procesa bytes y entrega eventos FIFO con la marca del LF recibido;
no accede todavía a la UART real ni envía telemetría. Cada llamada recibe un orden
asignado al recibir el byte, creciente y serializado externamente con los cambios
de modo. Buffers de trama/cola acotados; errores explícitos y recuperación.

El máximo común vive en Configuracion/Limites.h. Las USW de Comunicacion y la
constante están incluidas en el Doxyfile parcial. Ver
[especificación vigente](openspec/specs/comunicacion/spec.md)
y [verificación B02](openspec/changes/archive/2026-11-06-b02-recepcion-consignas/verify-report.md).
Con sanitizadores, ejecutar fuera de depurador/ptrace para que LeakSanitizer funcione.

## B03: cálculo lógico de velocidad

B03 original y revisión B03-R1 están aceptados/archivados. Revisión aceptada
el 2026-11-09; contrato vigente sincronizado. La API actual recibe pulsos, duración_us y
pulsos_por_vuelta uint32_t explícitos; no conoce el disco ni incluye su configuración.
El TPI mantiene 60 pulsos/vuelta en Configuracion/Medicion.h para un flanco/ranura.
Cambiar factor no requiere recompilar el servicio ni cambiar unidades para aplicación.

Salida: milésimas de RPM y estado. Cero pulsos con duración/factor positivos es cero
válido. Cercano/empate arriba, sin redondeo intermedio y error aritmético <=0,0005 RPM.
Prioridad de errores: NULL, duración cero, factor cero y resultado fuera de uint32_t.
Error escribible invalida y limpia; siguiente ventana válida recupera. No recorta
mediciones a 185 RPM. Captura/configuración coherentes y acceso serializado externos;
sin captura física, timeout ni main integrado.

23 casos C B03 y oráculo Python estándar de 11199 ternas pasan en host/sanitizadores.
Regresiones: 102 casos C totales; ARM Debug 8/8 y fixture B03 Release; Doxygen estricto.
Servicio y consumidor aislados sin configuración/dispositivo. Cálculo uint64_t
acotado y libgcc existente, sin libc en firmware. Compilar/enlazar no acredita
ejecución física. Ver [verificación B03-R1 aceptada](openspec/changes/archive/2026-11-09-b03-factor-pulsos-configurable/verify-report.md)
y [evidencia B03 original](openspec/changes/archive/2026-11-06-b03-medicion-velocidad/verify-report.md).

## B04: normalización del mando manual

Mandos.h/.c convierte lectura y fondo de escala explícitos (uint32_t) en mando
0..1000 por mil (uint16_t), independiente de bits, VREF o canal del ADC. Ejemplo:
2048/4095 produce 500, equivalente al 50 % del mando lógico. Redondeo cercano,
empate arriba; extremos 0 y fondo de escala producen exactamente 0 y 1000.
Inicializar deja SIN_DATOS. Escala cero o lectura excesiva invalidan y borran el
valor anterior; sólo VALIDO permite consumo. No implementa selector, ADC/DMA o PWM.

El límite manual tiene fuente común en Configuracion/Limites.h; Procesamiento
conserva su alias y comportamiento. Prueba/fixture adapta salidas válidas a B01;
no hay dependencia del servicio hacia aplicación ni integración del main real.
CTest agrega 15 casos B04 con recorridos, escalas equivalentes, errores y fronteras;
Doxygen documenta sus dos USW. Ver [informe B04](openspec/changes/archive/2026-11-06-b04-normalizacion-mando-manual/verify-report.md).

## B05: temperatura genérica y dispositivo LM35

Servicios/Temperatura.h/.c recibe temperatura ya expresada en centésimas Celsius
(int32_t) y estado de fuente válido/sin datos/error. Preserva valores válidos o
invalida con cero; diferencia cero válido de ausencia. No conoce el sensor concreto.
La implementación actual de Dispositivos/LM35.h/.c corresponde a B05-R1:
consulta muestra propia y devuelve centésimas, sin estados de dispositivo. Revisión
aceptada y sincronizada en el contrato vigente; TE2 del servicio se conserva. ADC/DMA físicos pendientes.

El servicio compila y enlaza por separado, sin LM35; el fixture de composición
entrega las centésimas de LM35_Read directamente, sin reescalar ni volver a redondear.
La composición con adquisición real se resolverá después, sin acceso directo a
dispositivos desde lógica de aplicación. ADC/DMA, alarma y main siguen pendientes.
Ver [informe corregido](openspec/changes/archive/2026-11-07-b05-conversion-temperatura/verify-report.md);
[informe inicial](openspec/changes/archive/2026-11-07-b05-conversion-temperatura/verify-report-inicial.md)
conserva la evidencia de la arquitectura anterior y no acredita la API actual.

## B06: evaluación lógica de alarma de temperatura

B06 aceptado y archivado el 2026-11-09; contrato vigente sincronizado. Nueva función
Procesamiento_EvaluarAlarmaTemperatura recibe la lectura genérica del servicio y
umbral int32_t en centésimas Celsius como parámetro de configuración, sin default.
Sólo una lectura válida superior al umbral produce ACTIVA; igual/inferior, INACTIVA.
Ausencia/error conservan categoría propia y no se presentan como temperatura normal.
Cada retorno es una decisión completa, sin memoria o inicialización adicionales.

La evaluación no modifica lectura, modos, mandos o consignas. Driver_Alarmas,
LED/Buzzer, prioridad temporal y main se integrarán en bloques posteriores.
La dependencia Aplicacion→Temperatura.h usa unidades/validez del servicio, sin LM35/ADC.
USW.2.1.2.7 documentada; Doxyfile existente cubre 22 funciones/USW sin avisos.

13 casos B06 + regresiones: 115 casos C y oráculo B03-R1 de 11199 ternas pasan
en host/sanitizadores. ARM Debug 9/9 y fixture B06 Release pasan sin ejecutar MCU.
Ver [contrato vigente](openspec/specs/alarma-temperatura/spec.md)
y [evidencia B06](openspec/changes/archive/2026-11-09-b06-logica-alarma-temperatura/verify-report.md).

## B07: coordinación portable de salidas de alarma

B07 original y B07-R1 aceptados/archivados; contrato vigente sincronizado
el2026-11-10, incluida la simplificación de las salidas. Driver_Alarmas_Aplicar recibe
activar/desactivar y una interfaz const con sólo dos funciones void(bool), sin contextos.
Valida toda la interfaz antes de actuar y llama LED→buzzer una vez cada uno.
Retorna sólo ACEPTADA (solicitud despachada) o ERROR_ARGUMENTO (interfaz NULL/
incompleta, sin llamadas). No diagnostica fallos físicos ni retiene resultados.

El servicio no incluye aplicación, dispositivos o biblioteca del MCU. La prueba
de composición conecta directamente LED_Write/Buzzer_Write sin adaptadores/casts.
Adaptación térmica B06 sólo emite para ACTIVA/INACTIVA; ausencia/error conserva
diagnóstico sin emitir apagado. Uso desde main/configuración física siguen pendientes.
Interfaz/funciones estables y acceso serializado externos; sin init/heap/esperas.

Ocho casos de servicio, cuatro térmicos y ocho GPIO; host/sanitizadores18/18,
183 casos C y oráculo Python. ARM Debug13/13 y fixture B07 Release válido, sin
ejecución en MCU. Doxygen38 funciones, USW.2.1.6.1 revisada, sin avisos.
Ver [evidencia B07-R1](openspec/changes/archive/2026-11-10-b07-r1-salidas-sin-contexto/verify-report.md).
Ver [contrato vigente](openspec/specs/salidas-alarma/spec.md); archivo B07 original
conservado. Uso desde aplicación/main queda para la integración final.

## B08: mando de motor portable

B08 aceptado/archivado el 2026-11-09; contrato vigente sincronizado. Driver_Motor_Aplicar recibe
uint16_t 0..1000 por mil y una interfaz función/contexto externa. Valida argumento y
rango antes de invocar una vez; rechaza excesos sin saturar ni emitir cero.
Driver_Motor_SolicitarCero solicita 0 mediante el mismo contrato. ACEPTADA/error de
argumento/rango/salida son resultados de esta solicitud, sin confirmación física.

Límite compartido con Mandos/B01. Servicio independiente de modos/RPM/MCU/dispositivos;
sin init/heap/globals mutables/reintentos/esperas propias. Proveedor/contexto vivos y
serializados, sin modificar interfaz. Pruebas componen B04→B01→Driver_Motor:
MANUAL usa mando válido, CERO solicita cero y AUTOMATICA no envía RPM como PWM.
main, PWM físico y regulador siguen pendientes.

10 casos nuevos del servicio y 5 integración: 146 C totales + 11199 ternas B03-R1,
host/sanitizadores 13/13, ARM Debug 11/11 y fixture B08 Release PASS.
Doxygen estricto: 25 funciones/USW, nuevas USW.2.1.8.1/.2. No hubo ejecución ARM/física.
Ver [contrato vigente](openspec/specs/mando-motor/spec.md)
y [verificación B08](openspec/changes/archive/2026-11-09-b08-mando-motor-portable/verify-report.md).
Cierre y publicación B08 autorizados al aceptar el resultado.

Plan de diagramas para el informe final: ver [CONTEXTO](docs/agente/CONTEXTO.md#documentación-final-y-b08).

## B09: codificación portable de telemetría

B09 aceptado/archivado el 2026-11-09; contrato comunicación sincronizado. Comunicacion recibe
una instantánea lógica propia: RPM en milésimas, temperatura en centésimas Celsius,
validez independiente y estado MANUAL/AUTO_ESPERA/AUTO_ACTIVO. Genera, por ejemplo:

```text
RPM=123.456;TEMP=25.34;ESTADO=MANUAL
```

LF termina la trama; NUL adicional no cuenta en longitud. ND/ERR representan
ausencia/fallo, distintos de cero válido. No se recortan mediciones a 185 RPM.
54 bytes de almacenamiento cubren el máximo; buffer corto/argumentos/enums inválidos
devuelven error explícito con longitud cero y destino vacío si permite escribir.
Sin trama parcial utilizable, heap, flotantes, libc nueva, UART o dependencia de aplicación.
Memoria coherente/no solapada y acceso serializado externos. Recepción B02 intacta;
composición B01/B03/B05 sólo en pruebas/fixture, sin modificar Procesamiento/main.

11 casos codec + cuatro integración: 161 C totales, 5096 pares contra snprintf host
y 11199 ternas B03-R1. Host/san 15/15, ARM Debug 12/12 y fixture B09 Release PASS;
Doxygen estricto 29 funciones/USW, nuevas USW.2.1.7.7/.8/.9/.10.
Ver [contrato vigente](openspec/specs/comunicacion/spec.md)
y [verificación](openspec/changes/archive/2026-11-09-b09-codificacion-telemetria/verify-report.md).
Contrato vigente conserva recepción B02 y agrega telemetría aceptada B09.
UART/envío/periodicidad/main físicos pendientes; sin ejecución ARM/simulador/placa.

## B10: selección estable de modo

B10 aceptado/archivado el 2026-11-09; contrato mandos sincronizado. Mandos agrega
InicializarSelector/ActualizarSelector con contexto por instancia, muestras de modo
lógico/validez y tiempo uint32_t ms externos. Intervalo configurable 0..INT32_MAX,
sin default TPI; cero confirma inmediatamente, otro valor exige estabilidad observada.
Cada rebote reinicia candidato; conservar confirmado mientras se califica otro.
Antes de confirmar o tras pérdida/error, modo NINGUNO y diagnóstico explícito.

Tiempo modular admite iguales/rollover con avances <2^31 ms; error temporal descarta
anclaje y exige recalificar en época nueva. Fuente desconocida no acepta tiempo;
fuente ausente/fallida y modo inválido conservan el timestamp aceptado. Sin GPIO,
reloj propio, esperas, heap o dependencia de aplicación. Acceso serializado externo.
Normalización B04 intacta; composición B01/B08 sólo en pruebas. Transición confirmada
manual→auto solicita cero y exige consigna nueva; rebotes no invalidan auto activo.

14 casos nuevos + cuatro integración: 179 C; referencia de 4096 muestras/400 rollovers,
5096 pares codec y 11199 ternas velocidad. Host/san 17/17; tras ampliar sólo aserciones
temporales se repitió selector host/san 1/1. ARM Debug 13/13 y fixture B10 Release PASS;
Doxygen estricto 32 funciones/USW, tres nuevas USW.2.1.5.3/.4/.5.
Ver [contrato vigente](openspec/specs/mandos/spec.md)
y [verificación](openspec/changes/archive/2026-11-09-b10-seleccion-estable-modo/verify-report.md).
MA3 vigente permite selector lógico B10 y conserva exclusiones de hardware.
GPIO/base temporal/main y pruebas físicas pendientes; sin ejecución ARM/simulador/placa.

## B11: wrappers Timer

B11 aceptado y archivado el 2026-11-10: `Dispositivos/Timer.h/.c` con tres funciones:

- `Timer_Init()`: configura TIMER0 mediante TIM_InitTimer, dejándolo detenido.
- `Timer_Start()`: inicia la cuenta mediante TIM_Enable.
- `Timer_Read()`: devuelve milisegundos mediante TIM_ReadTimer y TIMER_BASE_MS.

`TIMER_BASE_MS` está en Timer.h: default1ms, configurable al compilar. Se eliminó
Configuracion/Temporizacion.h. Header sólo con tipos estándar; detalles LPC1769 en .c.
Llamar Init y Start antes de Read; timer exclusivo desde reset, reloj informado
correcto y marcas de misma época. Servicios pueden restar lectura actual menos
marca inicial, con duraciones reales<2^31ms. No se agregó servicio Tiempo.

Driver original intacto. Host17/17 y ARM13/13 son regresiones previas; el nuevo
wrapper compila con headers reales y Doxygen35 funciones pasa sin avisos.
Las operaciones del timer no se ejecutaron en host ni placa. Main sigue mínimo;
enlace del conjunto de drivers y arranque real quedan para integración física.
Ver [contrato vigente](openspec/specs/timer/spec.md),
[verificación](openspec/changes/archive/2026-11-10-b11-wrapper-timer/verify-report.md) y
[cierre](openspec/changes/archive/2026-11-10-b11-wrapper-timer/archive-report.md).

## B12: dispositivos GPIO

Aceptado y archivado el2026-11-10, incluida la corrección de las salidas. Se completaron los módulos
existentes con llamadas directas a la biblioteca original:

- `Selector_Read()`: contacto en P2.6, activo por alto, mediante GPIO_ReadValue.
- `LED_Write(activar)`: rojoP0.22, activo por bajo, mediante GPIO_SetPinState.
- `Buzzer_Write(activar)`: P2.7, activo por alto, mediante GPIO_SetPinState.

Las salidas son void(bool), sin contexto ni retorno. La revisión B07-R1 ya
aceptada comprueba conexión directa con Driver_Alarmas en host; integración física pendiente.
Lectura del selector sin antirrebote/mapeo de modo. Dispositivos usa los drivers
originales directamente; el servicio se revisa por separado en B07-R1.

Evidencia al aceptar B12: host/sanitizadores18/18 (186 C), siete casos GPIO y regresiones; ARM13/13 y módulos
O0/O2 compilados, Doxygen38 funciones propias sin avisos.90 archivos Drivers originales
intactos. No ejecución física ni enlace de biblioteca; GPIO/PINSEL/dirección/polarización/
FIOMASK y main se configuran al integrar, que por instrucción del usuario queda al final.
Ver [contrato vigente](openspec/specs/dispositivos-gpio/spec.md) y
[verificación B12](openspec/changes/archive/2026-11-10-b12-dispositivos-gpio/verify-report.md).

## B07-R1: interfaz de alarmas simplificada

Aceptado/archivado2026-11-10. Dos funciones
void(bool), sin contextos ni errores de proveedor; validación previa conserva
ACEPTADA/ERROR_ARGUMENTO y orden LED→buzzer. Biblioteca y código B06/B12 intactos.
Se probaron composición directa y regresiones, sin inicialización/main/ensayo físico.
Ver [cambio](openspec/changes/archive/2026-11-10-b07-r1-salidas-sin-contexto/proposal.md) e
[informe](openspec/changes/archive/2026-11-10-b07-r1-salidas-sin-contexto/verify-report.md).

## B13: UART0 de dispositivos

B13 aceptado/archivado2026-11-10; contrato vigente sincronizado.
UART_Read/UART_Write delegan una vez a UART_Receive/Send con NONE_BLOCKING y
retornan cantidad real, permitiendo progreso parcial/cero. NULL/tamaño0 no accede
al driver ni buffer; sin protocolo, estado propio, heap, colas o ISR nuevos.

Asignación del manual UM10360 tabla80/p118: TXD0 P0.2 y RXD0 P0.3, función01.
UART_BAUDRATE en Dispositivos/UART.h, default115200, editable u override -D del
compilador; rechaza0/negativo/exceso uint32. El parámetro todavía no se aplica al
hardware: pinmux, baudios/reloj/formato/FIFO y conexión a Comunicacion/main al final.

Host/sanitizadores19/19,194 casos C+oráculo Python;11 casos UART nuevos. ARM13/13
y objetos UART-O0/O2 compilados, Doxygen40 funciones sin avisos.90 archivos Drivers
originales y otros módulos intactos. Sin ejecución MCU/simulación/ensayo eléctrico.
Ver [cambio](openspec/changes/archive/2026-11-10-b13-dispositivo-uart/proposal.md) y
[evidencia](openspec/changes/archive/2026-11-10-b13-dispositivo-uart/verify-report.md).

Ver [contrato UART vigente](openspec/specs/uart-dispositivo/spec.md).

## B14: lectura Sensor_Optico

B14 aceptado y archivado el 2026-11-10 para lectura acumulada; contrato sincronizado.
SensorOptico_Read devuelve conteo uint32_t acumulado mediante
TIM_ReadTimer(LPC_TIM1), sin reset/ventanas/RPM/ISR/estado propio. TIMER0/base y
servicio Medicion_Velocidad conservados; composición B03 demostrada sólo en host.

Sensor actual confirmado por usuario: XD-51, reemplaza HC-89 del anteproyecto.
No se presuponen niveles/alimentación/pinout del módulo. Pinmux/contador y señal
eléctrica, ventana1s y conexión a servicios/main se dejan para integración final.

Host/sanitizadores20/20,200 casos C y oráculo Python;6 casos B14 nuevos. ARM13/13
y objetoO0/O2 compilados, Doxygen41 funciones sin avisos. Drivers90 originales y
módulos/contratos aceptados intactos; sin ejecución MCU/ensayos físicos.
Ver [cambio](openspec/changes/archive/2026-11-10-b14-sensor-optico-contador/proposal.md) y
[evidencia](openspec/changes/archive/2026-11-10-b14-sensor-optico-contador/verify-report.md).

Ver [contrato vigente](openspec/specs/sensor-optico/spec.md).

## B15: dispositivo Motor

Por orden del usuario, MR0 queda fijado en1000 ticks para la inicialización final.
Motor_Write recibe sólo mando0..1000, valida y despacha directamente
PWM_MatchUpdateSingle(PWM_MATCH_1,mando_por_mil), incluido1000 sin ajustar a1001.
Sin cálculo de match, lectura MR0, conversión/64bits o modelo de registros en pruebas.
false rechaza >1000 sin escritura; true indica despacho, no efecto eléctrico.

B15 aceptado/archivado2026-11-10; contrato motor-dispositivo sincronizado.
Host/san21/21,204 casos C+oráculo;
cuatro casos B15,1001 mandos válidos/64535 rechazos. ARM13/13,O0/O2 y Doxygen42
sin avisos. Drivers/servicios/otros dispositivos/contratos aceptados intactos.
Inicialización/frecuencia/potencia/adaptación B08/main y validación física PWM.1
quedan para el final. Sin ejecución MCU/placa ni push.
Ver [cambio](openspec/changes/archive/2026-11-10-b15-dispositivo-motor-pwm/proposal.md) y
[evidencia actual](openspec/changes/archive/2026-11-10-b15-dispositivo-motor-pwm/verify-report.md).

Ver [contrato de Motor vigente](openspec/specs/motor-dispositivo/spec.md).

## B16: lectura del potenciómetro

uint16_t Pot_Read(void) devuelve0..4095 desde ADC_DMA_Pot, definida en Potenciometro.c,
usando ADC_GDR_RESULT original. Sin parámetros, estados o validaciones adicionales.
Canal AD0.0/P0.23 función01 acordado; ADC convierte y DMA será el productor de esa
memoria al integrar. El extern interno permite configurar su dirección como destino.
La prueba enlaza sin Adquisicion_ADC_DMA.c. Configuración, arranque y conexión
a servicios/main al final.

B16 aceptado y archivado el 2026-11-11; contrato potenciometro-dma sincronizado.
Host/san22/22 (207 casos C y
oráculo), ARM13/13 y objetosO0/O2, Doxygen43 sin avisos. Drivers90 originales,
servicios/otros dispositivos/contratos aceptados intactos. Sin ejecución MCU/placa.
Cierre documental con commit local; sin push. Sin repetir pruebas al archivar.
Ver [alcance](openspec/changes/archive/2026-11-11-b16-lectura-potenciometro-dma/proposal.md) y
[evidencia](openspec/changes/archive/2026-11-11-b16-lectura-potenciometro-dma/verify-report.md).

Ver [contrato vigente del potenciómetro](openspec/specs/potenciometro-dma/spec.md).

## B05-R1: lectura LM35 aceptada y archivada

int32_t LM35_Read(void) devuelve centésimas desde ADC_DMA_LM35 propia, sin estados
de dispositivo; redondeo B05 cercano/empate arriba. Constantes con nombre/derivadas,
referencia ADC_VREF_MV #ifndef/default2540mV provisional/#error, sin atribuirlo a placa.
Test enlazado sólo con LM35.c; sin Adquisicion_ADC_DMA.c.

Resultado 94d3134 aceptado por el usuario el 2026-11-11; reauditoría de Claude r2:
APROBAR, sin hallazgos abiertos. TE1/TE3/TE4 sincronizados; TE2/S4/S5 intactos.
Verificación propia repetida al cerrar: host/sanitizadores29/29 cada uno,
214 ejecuciones C,20480 comparaciones racionales exactas y8 probes por modalidad;
ARM13/13 y Doxygen43 sin avisos. Código, pruebas, build y Drivers sin cambios en el cierre.
Inicialización ADC/DMA, VREF física e integración al final. Sin pruebas físicas ni push.
Ver [cambio archivado](openspec/changes/archive/2026-11-11-b05-r1-lectura-lm35/proposal.md),
[evidencia de implementación](openspec/changes/archive/2026-11-11-b05-r1-lectura-lm35/verify-report.md),
[cierre y comprobaciones nuevas](openspec/changes/archive/2026-11-11-b05-r1-lectura-lm35/archive-report.md) y
[resumen portable de revisiones externas locales](openspec/changes/archive/2026-11-11-b05-r1-lectura-lm35/review-notes.md).


## B17: regulador automático implementado, pendiente de auditoría

Procesamiento agrega InicializarRegulador/Regular/ReiniciarRegulador separados
de B01, con PI entero y salida0..1000, saturación/anti-windup y retención limitada
sin integrar sobre dato viejo. Control.h configura ganancias/N;500ms contractual,
sin Timer. No integra main/hardware.

Host/san33/33 cada uno, oráculo116722 pasos, planta14/14 pasos, ARM15/15 y
Doxygen46 sin avisos. Mutantes AW/truncado detectados. B01/B06/Drivers intactos.
Evidencia workspace build/verificacion-b17-2026-11-11 (ignorada); reproducir desde
raíz con `python3 scripts/verificar_b17.py`.
[Contrato candidato](openspec/changes/b17-regulador-velocidad/specs/regulacion-velocidad/spec.md)
e [informe](openspec/changes/b17-regulador-velocidad/verify-report.md).
Pendiente de /auditar B17 y aceptación; sin cierre/archivo/push.

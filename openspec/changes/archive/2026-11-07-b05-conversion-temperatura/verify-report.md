# Verificación B05 corregido — 2026-11-07

Resultado: **PASS para el alcance corregido**, pendiente de aceptación. Base:
fcd43927d7b2dbaed19169d4cfb249de4a3da16f. Commit correctivo con asunto
`fix: desacoplar Temperatura del sensor LM35`; consultar Git para hash. Sin push.
La aclaración actual del usuario prevalece sobre el diseño candidato inicial:
los servicios también deben ser independientes del hardware específico.

## Problema reproducido y cambio

La curva 10 mV/°C y parámetros ADC estaban en Servicios/Temperatura; LM35.h delegaba
la conversión al servicio. Un consumidor con Celsius y estado genérico no compilaba
contra esa API. Se escribió primero el consumidor actual de test_temperatura.c y
se compiló contra fcd4392: faltaban Temperatura_EstadoFuente/FUENTE_VALIDA y el contrato
de llamada. Fallo esperado guardado en OUT/reproduccion-acoplamiento.log.
El mismo consumidor ahora compila/enlaza con sólo Temperatura.c y ejecuta 10/10 casos.

Dispositivos/LM35 ahora contiene inicialización de lectura y conversión ADC→Celsius,
con ganancia/escala/errores numéricos propios. Servicio Temperatura recibe int32_t
centésimas Celsius y fuente VALIDA/SIN_DATOS/ERROR, conservando valor válido o
invalidando con cero. No contiene LM35, ganancia, muestra ADC, escala o referencia.
No incluye ni enlaza el dispositivo. Dispositivo tampoco depende del servicio.
La adaptación se prueba en código de integración/fixture, no en main de producción.
No se añade lectura física, callback, selección de fuente o configuración de periféricos.

## Matriz de aceptación

| Req. / escenario | Evidencia ejecutada | Resultado |
|---|---|---|
| TE1 / S1 referencias | test_lm35: ReferenciasNominales; integración: ConversionYPropagacion | PASS |
| TE1 / S2 equivalencia/escala | test_lm35: ConfiguracionesEquivalentes, EscalaYUltimoCodigo | PASS |
| TE1 / S3 redondeo/límites | test_lm35: RedondeoYEmpates, SinRedondeoIntermedio, ExtremosDeTipos, matriz/barrido/recorridos | PASS |
| TE2 / S4 Celsius/disponibilidad | servicio: Inicializacion, FuentesEnUnidadesFisicas, CeroYDisponibilidad, Reinicio; integración: AusenciaYCeroValido | PASS |
| TE2 / S5 invalidación/recuperación | servicio: ErrorInvalidaDatoAnterior, AusenciaInvalidaDatoAnterior, Recuperacion; integración: ErrorYRecuperacion | PASS |
| TE3 / S6 argumentos/fronteras | servicio: ArgumentosInvalidos, int32 MIN/MAX; dispositivo: prioridades y uint32 MAX; adaptación con cota explícita | PASS |
| TE3 / S7 aislamiento | servicio: InstanciasIndependientes, SecuenciaDeFuentes; consumidor y ELF sin LM35; inspección de dependencias | PASS |
| TE4 / S8 evidencia | CTest, sanitizadores, ARM Debug/Release, Doxygen y diff descritos abajo | PASS |

Conversión LM35 conserva exactamente los cuerpos de las dos funciones iniciales,
salvo nombres de símbolos/estados. Comparación textual normalizada contra fcd4392
ejecutada; no se cambió su algoritmo. Los 17 casos numéricos siguen en test_lm35.c,
incluidos 1000 fronteras, 10000 ternas y 70678 puntos de recorrido.
El servicio tiene 10 casos independientes, con fuentes emuladas en unidades físicas,
valores negativos/INT32_MIN/MAX, ausencia, fallo, argumento inválido y recuperación.
Tres casos de integración verifican adaptación, cota uint32→int32, errores y cero válido.

## Comandos y resultados reales

OUT=/tmp/ed3-b05-desacople-20261107-ltmr1lup. Herramientas existentes; C11 y advertencias como errores, sin controles desactivados.

| Configuración/comando | Resultado |
|---|---|
| `cmake -S . -B "$OUT/host/build" -DTPI_BUILD_ARM=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug`; build; `ctest --test-dir "$OUT/host/build" -V --output-on-failure` | CTest 7/7; 64 B01–B04 + 10 servicio + 17 LM35 + 3 integración = 94 casos |
| Mismo flujo en `$OUT/san/build` con `-DTPI_HOST_SANITIZERS=ON` | 94 casos sin hallazgos ASan/LeakSanitizer/UBSan |
| Sanitizadores fuera de sandbox/ptrace: `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest ...` | Sin suprimir fallos |
| ARM Debug con `-DTPI_BUILD_ARM=ON -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake" -DCMAKE_BUILD_TYPE=Debug`; build/CTest | 8/8; B00 conserva sus 14 casos, fixtures B01–B04 y dos B05 |
| ARM Release build y `ctest --test-dir "$OUT/arm-release/build" -R '^b05\.' -V --output-on-failure` | 2/2; servicio aislado y LM35+servicio correctos a -O3 |
| `cc -std=c11` con avisos estrictos, `-IServicios`, consumidor genérico + Servicios/Temperatura.c, sin directorio ni fuente de dispositivos | Compila/enlaza y pasa 10/10; antes fallaba con mismo consumidor |
| `doxygen -`, Doxyfile versionado y OUTPUT_DIRECTORY temporal; inspección XML | Sin avisos; cuatro USW B05 y 21 funciones totales |
| ARM `-O0 -ffreestanding -fstack-usage` para ambos módulos; size/nm/objdump | Recursos abajo; servicio sin símbolos indefinidos, LM35 usa libgcc |
| Hashes iniciales, cuerpos LM35 normalizados, vínculos/YAML y diff/índice | Correctos; B00–B04 e historial aceptado preservados |

Pruebas actuales sin fallos pendientes del alcance. La evidencia inicial de 81 casos
y controles negativos numéricos se conserva sin modificar en verify-report-inicial.md;
corresponde a fcd4392 y no acredita la interfaz actual. No se afirman controles
negativos reejecutados ahora: el algoritmo trasladado es idéntico y la suite se recompiló.
Cobertura sin umbral configurado; no se ejecutó medición de cobertura. Sin CI nuevo.

## Recursos estáticos y Doxygen

| Unidad ARM -O0 | Código / datos | Pila estática / símbolos |
|---|---|---|
| Servicio Temperatura | .text 142 bytes, .data/.bss 0; lectura 8 bytes | Inicializar 16, Actualizar 24 bytes; sin auxiliares externos |
| Dispositivo LM35 | .text 390 bytes, .data/.bss 0; lectura 8 bytes | InicializarLectura 16, ConvertirMuestra 128; cadena con libgcc hasta 176 bytes |
| Fixture servicio Debug / Release | Flash 1072 / 968; SRAM 2064 con pila reservada 2048 | Enlace sin LM35, libc ni libgcc |
| Fixture compuesto Debug / Release | Flash 2276 / 1876; SRAM 2076 con pila reservada 2048 | APIs y __aeabi_uldivmod retenidos, sin indefinidos |

Ambos ELF pasan verificador B00 y GNU_STACK RW. Pila interna LM35: 128+16+32,
excluye llamador/IRQ; no es medición física. Servicio/dispositivo se ejecutan por
etapas, sin añadir cola, heap, espera, recursión o globals mutables.
Doxygen: USW.2.1.4.1/.2 y USW.2.1.11.1/.2 documentadas con parámetros, retornos,
unidades, errores y concurrencia; sin comentarios dentro de cuerpos de producción.

## Fuente y limitaciones

Fuente local aportada por el usuario, TI SNIS159H, §7.3.1 p.13; ecuación comprobada
en texto y render. El render emitió avisos de fuentes/caché; la ecuación es legible,
se contrastó con extracción textual y no se modificó el PDF. Procedencia registrada:

/home/luis/Desktop/Universidad/ed3/Datasheet_LM35_TI.pdf
SHA-256 2f249badf3c1c9036b5fdff21ab26b097df9488f7cb6accd58666bb965074087

Servicio genérico acepta Celsius negativos ya proporcionados por otra fuente; esto
no implementa circuito/offset negativo del LM35. LM35 mantiene modelo nominal sin
offset y tipos/cotas de B05 inicial. Captura coherente y acceso serializado siguen
siendo externos. No hay ADC/DMA real, integración de adquisición/main, VREF/pines,
validación eléctrica, frescura, alarma o ejecución MCU/simulador.

R15 se aplica transversalmente: B03 conserva 60 pulsos/vuelta en un contrato ya
aceptado y requiere revisión posterior. No se modifica aquí ni se declara toda la
capa de servicios independiente del hardware. No se cambiaron archivos archivados.
B05 queda verificado pero pendiente de aceptación, sin sincronización/archivo ni push.
Próxima acción: revisar aceptación del B05 corregido; no iniciar otro bloque.

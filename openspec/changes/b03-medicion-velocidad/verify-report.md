# Verificación B03 — 2026-11-06

Resultado: **PASS para el alcance lógico acordado**, pendiente de aceptación del
usuario. Base main: 27ad99a7d04c52dfd98344857fa5c98ed7ad61d4. Commit local de cierre
con asunto `feat: implementar B03 de medicion de velocidad`; consultar Git para hash.
Sin push autorizado; contratos vigentes y archivos B00–B02 conservados.

## Comportamiento y conformidad

Medicion_Velocidad convierte conteo uint32_t y duración uint32_t en microsegundos,
para 60 pulsos/vuelta del eje portadisco. Entrega milésimas de RPM y estado explícito.
Redondeo cercano con empates hacia arriba; máximo error aritmético 0,0005 RPM.
Una lectura puede superar 185 RPM. La API tiene dos funciones, sin heap, flotantes,
estado global, dispositivos, bloqueos ni integración de main. Escritura/lectura y
captura coherente requieren serialización externa. No evalúa frescura ni overflow
previo del capturador; esos requisitos físicos siguen pendientes.

| Req. / escenario | Evidencia ejecutada en tests/host/test_medicion_velocidad.c | Resultado |
|---|---|---|
| MV1 / S1 referencias y cero | Referencias, SinRecorteDeConsigna: 0/60/185/200 RPM y 186/1000 sin recorte | PASS |
| MV1 / S2 unidades y redondeo | VentanasEquivalentes, Redondeo (arriba/abajo/empate), CeroRedondeadoValido | PASS |
| MV2 / S3 ausencia/reinicio | AusenciaInicial, Reinicio: SIN_DATOS distinto de cero válido; reinicio de válida/error | PASS |
| MV2 / S4 duración/recuperación | DuracionNulaYRecuperacion: dato viejo borrado, cero/UINT32_MAX con duración cero | PASS |
| MV2 / S5 extremos/rango | RangoYRecuperacion, ExtremosDeTipos, DuracionMinima; max representable y exceso | PASS |
| MV3 / S6 argumentos/aislamiento | PunterosNulos, InstanciasIndependientes; todas las salidas reemplazadas | PASS |
| MV4 / S7 verificación | CTest host/ARM, sanitizadores, enlace -O0/-O3 y Doxygen descritos abajo | PASS |

Refuerzo MV1/MV2: MatrizDeFronteras (100 combinaciones) y BarridoDeterminista
(10 000 ventanas) comprueban distancias enteras al valor exacto y al límite, sin
calcular el cociente pulsos/tiempo ni repetir el algoritmo de producción. Se verifica también que el valor real
ligeramente superior a UINT32_MAX milésimas puede redondear al máximo representable:
4 304 180 pulsos/1 002 145 us → UINT32_MAX; con 1 002 144 us → ERROR_RANGO.
SIN_DATOS es estado tras inicio/reinicio; duración positiva, incluso cero redondeado,
es VALIDA. Errores de duración/rango borran valor anterior y permiten recuperación.

## Comandos y resultados reales

Desde raíz del proyecto; OUT indica el directorio temporal de este informe.
No se omiten ni desactivan controles. Sin dependencias instaladas.

| Comando / configuración | Resultado |
|---|---|
| `cmake -S . -B "$OUT/host/build" -DTPI_BUILD_ARM=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug`; build; `ctest --test-dir "$OUT/host/build" -V --output-on-failure` | CTest 3/3; B01 17 + B02 17 + B03 15 = 49 casos |
| Mismo flujo en `$OUT/san/build` agregando `-DTPI_HOST_SANITIZERS=ON` | 49 casos; ASan/LeakSanitizer/UBSan sin hallazgos |
| Sanitizadores fuera de sandbox/ptrace: `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest ...` | Sin controles suprimidos |
| Build y CTest ARM Debug: `-DTPI_BUILD_ARM=ON -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake" -DCMAKE_BUILD_TYPE=Debug` | CTest 5/5; incluye B00 14 casos negativos/positivos y fixtures B01/B02/B03 |
| Build ARM Release y `ctest --test-dir "$OUT/arm-release/build" -R b03.enlace_arm -V --output-on-failure` | API B03 completa retenida y enlazada a -O3 |
| `doxygen -` con Doxyfile versionado y OUTPUT_DIRECTORY temporal; inspección XML | Sin avisos; USW.2.1.3.1/.2 presentes, 15 funciones totales |
| `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 -O0 -ffreestanding -fstack-usage -c Servicios/Medicion_Velocidad.c -IServicios`; size/nm/objdump | Recursos indicados abajo; sin global mutable |
| `git diff --check`, revisión del diff/índice, hashes contra inventario inicial | Sin espacios inválidos ni cambios ajenos; archivos aceptados preservados |

Las dos aserciones finales de representabilidad redondeada se agregaron tras la
primera corrida: se volvió a compilar/ejecutar sólo B03 en host y sanitizadores;
15/15 PASS en ambos. Ninguna lógica de producción cambió tras la corrida general.
No hay umbral de cobertura configurado; no se ejecutó medición de cobertura.

## Enlace y recursos

Compiladores instalados GCC host 13.3.0 / ARM 13.2.1; C11 y advertencias estrictas
como errores. Doxygen 1.9.8. División de 64 bits resuelta con libgcc del perfil
thumb/v7-m/nofp ya instalado, usando -lgcc después de objetos y sin libc.
Fixture comprueba ambas funciones y __aeabi_uldivmod en símbolos definidos.
Verificador ELF B00 pasa; `nm -u` del fixture queda vacío.

El primer enlace falló porque _dvmd_tls.o de libgcc carece de nota GNU-stack.
Se resolvió declarando `-z noexecstack`, con --fatal-warnings conservado;
readelf confirma GNU_STACK RW tanto en firmware como en fixture. Un fixture
temporal con -z execstack fue rechazado por el nuevo control (fallo esperado).
Esto es metadata ELF: no configura el MPU ni acredita protección física de la pila.
El primer Doxygen detectó un delimitador @addtogroup mal formado; se corrigió
y la generación completa posterior pasó, sin suprimir avisos.

| Recurso ARM | Medición estática |
|---|---|
| Lectura | 8 bytes, sin buffers/heap |
| Objeto B03 a -O0 | .text 236 bytes, .data/.bss 0; único auxiliar pendiente antes de enlazar: __aeabi_uldivmod |
| Stack propio -O0 (-fstack-usage) | Inicializar 16 bytes; Actualizar 80 bytes |
| Auxiliar instalado (disassembly) | __aeabi_uldivmod reserva 16 bytes; __udivmoddi4 guarda 32, sin recursión |
| Cadena interna B03 -O0 | Hasta 128 bytes (80+16+32), excluye llamador/IRQ; no es medición física |
| Fixture B03 Debug / Release | Huella flash 1916 / 1740 bytes; SRAM 2064 (8 datos de fixture + 8 lectura + 2048 pila) |
| main mínimo Debug / Release | 844 / 840 bytes flash; SRAM 2048. GC elimina API no integrada; por eso se retiene en fixture |

Producto máximo más término de redondeo: 4 294 967 297 147 483 647 <UINT64_MAX.
División sólo tras duración>0; estrechamiento sólo tras resultado<=UINT32_MAX.
El cuerpo de las funciones de producción no contiene comentarios; contratos,
parámetros, unidades, errores, concurrencia y IDs se documentan fuera del cuerpo.

Referencias consultadas para enlace: [GCC -nostdlib/-lgcc](https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html)
y [GNU ld noexecstack](https://sourceware.org/binutils/docs/ld/Options.html).

## Revisión y límites

Sin discrepancias pendientes con el alcance/diseño. Contrato nuevo candidato:
no se sincronizó openspec/specs ni se archivó B03. main, Procesamiento,
Comunicacion, drivers/dispositivos, pruebas B01/B02 e historial aceptado intactos.
Todas las tareas técnicas cerradas; creación del commit autorizada, sin push.
Sin ejecución ARM, simulación ni prueba física: faltan adquisición, reloj, ventana,
un flanco por ranura, eje portadisco, pines, revisión PCB/silicio y precisión real.
No se flasheó ni cambió configuración del MCU. Próxima acción: aceptación de B03;
después sincronizar/archivar sólo si se solicita. No iniciar B04 automáticamente.

Logs extensos y artefactos: /tmp/ed3-b03-20261106-07i2qp2_ (temporales, fuera de Git).

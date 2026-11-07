# Verificación B05 — 2026-11-07

Resultado: **PASS para el alcance numérico acordado**; pendiente de aceptación del
usuario. Base main: 8d5bc7b040fc205af48a00db35de5f7622c82379. Cierre con asunto
`feat: implementar B05 de conversion de temperatura`; hash consultable en Git.
Commit autorizado, sin push. Se incorpora propuesta/contexto locales previos.

## Contrato y completitud

12 tareas de contrato, implementación, verificación y cierre. Temperatura ESW.2.1.4
implementa Inicializar (USW.2.1.4.1) y Actualizar (USW.2.1.4.2). Muestra, escala_adc
y referencia_uv son uint32_t. Escala significa cuentas por referencia, muestra<escala;
ejemplo 4096 admite 0..4095. No es el máximo inclusivo usado por Mandos B04.
Salida uint32_t en centésimas de °C, redondeada al más cercano con empate arriba;
sin redondeo de tensión intermedio ni suma que pueda desbordar el numerador.

Inicializar deja SIN_DATOS/cero; Actualizar reemplaza todo sin leer estado previo.
Prioridad: NULL → ERROR_ARGUMENTO, escala cero → ERROR_ESCALA, referencia cero →
ERROR_REFERENCIA, muestra>=escala → ERROR_MUESTRA. Error escribible invalida y borra
valor anterior. Entrada válida posterior recupera, incluso cero ideal válido.
Sólo VALIDA habilita consumo; indica conversión numérica, no plausibilidad eléctrica.
Modelo sin offset/no negativo; no supone variante/rango real del LM35 ni VREF de placa.

## Matriz de conformidad

Pruebas ejecutadas en tests/host/test_temperatura.c; no basta el análisis estático.

| Req. / escenario | Funciones de prueba ejecutadas | Resultado |
|---|---|---|
| TE1 / S1 referencias/escala | ReferenciasNominales, EscalaYUltimoCodigo, RecorridosMonotonos | PASS |
| TE1 / S2 equivalencia | ConfiguracionesEquivalentes, RecuperacionYCambioDeConfiguracion | PASS |
| TE1 / S3 redondeo único | RedondeoYEmpates, SinRedondeoIntermedio, MatrizDeFronteras, BarridoDeterminista | PASS |
| TE2 / S4 disponibilidad/reinicio | AusenciaYCeroValido, Reinicio | PASS |
| TE2 / S5 errores/recuperación | EscalaCeroInvalida, ReferenciaCeroInvalida, MuestraExcesivaInvalida, RecuperacionYCambioDeConfiguracion, PunterosNulosYPrioridad | PASS |
| TE3 / S6 límites | ExtremosDeTipos, MatrizDeFronteras, BarridoDeterminista | PASS |
| TE3 / S7 aislamiento | InstanciasIndependientes; inspección de dependencias/símbolos | PASS |
| TE4 / S8 evidencia | CTest host/ARM, sanitizadores, Release, Doxygen y controles negativos descritos abajo | PASS |

17 casos B05 incluyen 1000 ternas de fronteras, 10000 ternas deterministas mixtas y
70678 puntos de recorrido completo para escalas 1/2/3/16/1024/4096/65536. Oráculo
independiente de intervalos exactos alrededor de la salida: no recalcula N/D. Usa
(q−1)×D y verifica las fronteras inferior inclusiva/superior exclusiva del redondeo;
evita q×D, que puede exceder uint64_t aun con resultado correcto. Producto base
acotado tras exigir q<=42949673; sin extensiones __int128 ni flotantes en pruebas.

Referencias: 125/1500 y 500/1500 con 3000000 µV →2500 y 10000 centésimas.
Configuraciones equivalentes dan 2500. Muestra 1/4 con referencias 4999/5000/5001 µV
→12/13/13. Extremo MAX−1/MAX/MAX →42949673; muestra=escala se rechaza.

## Comandos y resultados reales

Desde raíz; OUT=/tmp/ed3-b05-20261107-e27t9yaq. GCC host 13.3.0, GCC ARM 13.2.1, CMake 3.28.3, Doxygen 1.9.8.
C11 y advertencias estrictas como errores; sin dependencias instaladas.

| Comando/configuración ejecutados | Resultado |
|---|---|
| `cmake -S . -B "$OUT/host/build" -DTPI_BUILD_ARM=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug`; build; `ctest --test-dir "$OUT/host/build" -V --output-on-failure` | 5/5 CTest; 17 B01 + 17 B02 + 15 B03 + 15 B04 + 17 B05 = 81 casos |
| Igual flujo en `$OUT/san/build`, agregando `-DTPI_HOST_SANITIZERS=ON` | 81 casos, sin hallazgos ASan/LeakSanitizer/UBSan |
| Sanitizadores fuera de sandbox/ptrace: `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest ...` | Controles completos; sin suprimir errores |
| ARM Debug en `$OUT/arm-debug/build`, con `-DTPI_BUILD_ARM=ON -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake" -DCMAKE_BUILD_TYPE=Debug`; build/CTest | 7/7; B00 incluye 14 casos y fixtures B01–B05 |
| ARM Release build y `ctest --test-dir "$OUT/arm-release/build" -R b05.enlace_arm -V --output-on-failure` | API B05 completa retenida y enlace -O3 correcto |
| `doxygen -` con Doxyfile versionado y OUTPUT_DIRECTORY temporal; inspección XML | Sin avisos; dos USW nuevas y 19 funciones totales, constantes documentadas |
| `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 -O0 -ffreestanding -fstack-usage -c Servicios/Temperatura.c -IServicios`; size/nm/objdump | Recursos abajo; sin globals mutables |
| Variantes temporales compiladas con `cc -std=c11 -O0` y mismos avisos estrictos, contra test_temperatura.c | Dos defectos detectados por la suite; detalle abajo |
| Diff/índice, hashes contra inventario inicial, YAML y vínculos locales | Correctos, sin cambios ajenos ni archivos aceptados modificados |

Controles negativos, sólo en OUT/controles-negativos/; no cambian fuentes del proyecto:

- `(N+D/2)/D` compila y falla como se esperaba en **S6 extremos uint32**: la suma
  envuelve para una entrada válida. Resultado de test exit=1 esperado, sin fallo oculto.
- Redondear primero a µV enteros compila y falla en **S3 redondeo/empates**; se detecta
  el doble redondeo. Exit=1 esperado. La implementación original pasa 17/17.

Sin fallos pendientes del bloque. Cobertura sin umbral configurado; no se ejecutó
medición de cobertura. No hay CI incorporado. Logs extensos/artefactos sólo en OUT.

## Aritmética, recursos y enlace ARM

N=muestra×referencia_uv <=18446744060824649730 en dominio válido. D=escala_adc×100
<=429496729500, positivo/par. Se calculan cociente/resto; resto>=D/2 incrementa
cociente. No se suma D/2 a N. Salida<=42949673, estrechamiento a uint32_t seguro.
Libgcc existente resuelve __aeabi_uldivmod, sin libc. Fixture comprueba ambas API,
auxiliar y ELF B00; `nm -u` vacío en Debug/Release y GNU_STACK RW verificado.

| Recurso ARM | Inspección estática |
|---|---|
| Temperatura_Lectura | 8 bytes; sin buffers/heap |
| Objeto Temperatura -O0 | .text 390 bytes, .data/.bss 0; único auxiliar pendiente antes de enlace: __aeabi_uldivmod |
| Stack propio -O0 (-fstack-usage) | Inicializar 16 bytes; Actualizar 128 bytes |
| Cadena interna -O0 | 128 + 16 (__aeabi_uldivmod) + 32 (__udivmoddi4) = hasta 176 bytes; excluye llamador/IRQ |
| Llamadas de división | Debug emite dos llamadas secuenciales al auxiliar; Release combina en una. No duplica pila simultánea |
| Fixture B05 Debug / Release | Huella flash 2080 / 1776 bytes; SRAM 2068 (12 datos de fixture + 8 salida + 2048 pila reservada) |
| main mínimo Debug / Release | Flash 844 / 840 bytes; SRAM 2048. GC elimina API sin integrar, por eso se retiene en fixture |

Sin recursión ni globals mutables; sin espera ni acceso a periféricos. La inspección
no acredita profundidad de pila, tiempos o ejecución reales. GNU_STACK es metadata,
no configuración del MPU. Adquisición y lectura/escritura requieren serialización externa.

## Fuentes, revisión y límites

Modelo nominal: [TI LM35 §7.3.1](https://www.ti.com/lit/ds/symlink/lm35.pdf).
Escala por cuentas: [NXP, fig.28, p.68](https://www.nxp.com/docs/en/data-sheet/LPC1769_68_67_66_65_64_63.pdf),
LSB ideal=(VREFP−VREFN)/4096. Se usa código×LSB sin corrección de media cuenta;
cuantización, precisión del ADC/LM35 y VREF real no forman parte del error aritmético.
No se asigna una referencia física ni se confirma variante/rango eléctrico del LM35.

Diseño seguido sin desviaciones. USW con parámetros, retornos, unidades, errores y
concurrencia Doxygen; sin comentarios dentro del cuerpo de funciones de producción.
Preservados main, B01–B04, Configuracion, drivers/dispositivos, pruebas anteriores,
contratos vigentes y archivos archivados. Propuesta/contexto previos incorporados.
Estado/spec B05 activos y candidatos, sin sincronización ni archivo antes de aceptación.
Commit local autorizado, sin push. Próxima acción: aceptación de B05; no iniciar B06.

Sin ejecución ARM, simulación ni hardware. Pendientes ADC/DMA, canales/pines, VREF,
variante/conexión del LM35, calibración/offset para negativos, filtro/frescura, umbral,
histéresis/alarma, UART y main. No se flasheó ni cambió configuración del MCU.

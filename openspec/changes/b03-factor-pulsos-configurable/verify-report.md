# Verificación B03-R1 — 2026-11-07

Resultado: **PASS para alcance lógico**, pendiente de aceptación. Base publicada
a11a97c1c8617fdc42007afa78d071e83b553512. Commit local con asunto
`fix: parametrizar pulsos por vuelta en medicion de velocidad`; consultar Git para hash.
No sincronizar/archivar ni publicar esta revisión antes de nueva autorización.

## Alcance, reproducción y coherencia

R02 especifica disco de 60 ranuras. R15 aclarado por el usuario exige servicios
independientes del hardware, y su orden actual autoriza revisar B03 por separado.
La API anterior no permitía indicar otro factor: el consumidor actual de cuatro
argumentos se escribió antes del cambio y falló al compilar contra a11a97c.
Log reproduccion-api.log: demasiados argumentos y falta del error de factor.
El mismo archivo, sin cambios, ahora compila/enlaza con sólo -IServicios y el .c
del servicio, sin configuración, dispositivo, aplicación ni hardware.

Factor positivo explícito por llamada, 60 del TPI en Configuracion/Medicion.h.
No hay wrapper/alias físico en el servicio. SEGUNDOS_POR_MINUTO=60 es conversión
universal de tiempo y no describe al disco. Se conserva API de inicialización,
lectura de dos campos, todos los códigos previos y dos IDs USW; se añade al final
ERROR_PULSOS_POR_VUELTA. Todos los consumidores activos migrados a cuatro argumentos.
main, aplicación, dispositivos y contratos/archivos aceptados permanecen intactos.

## Matriz de aceptación — 10/10 escenarios

| Req./escenario | Pruebas ejecutadas y evidencia | Resultado |
|---|---|---|
| MV1/S1 referencias TPI | Referencias, SinRecorteDeConsigna, configuración=60 | PASS |
| MV1/S2 equivalencia/redondeo | VentanasEquivalentes, Redondeo, CeroRedondeadoValido; oráculo | PASS |
| MV2/S3 ausencia/reinicio | AusenciaInicial, Reinicio | PASS |
| MV2/S4 duración/recuperación | DuracionNulaYRecuperacion | PASS |
| MV2/S5 extremos TPI | ExtremosDeTipos, DuracionMinima, RangoYRecuperacion, MatrizDeFronteras, BarridoDeterminista | PASS |
| MV3/S6 NULL/instancias | PunterosNulos, InstanciasIndependientes, InstanciasConFactoresDistintos | PASS |
| MV4/S7 evidencia/aislamiento | CTest, consumidor independiente, ARM, Doxygen y análisis de dependencias | PASS |
| MV1/S8 factores distintos | FactoresConfigurables, EscalasEquivalentes; oráculo 1/12/60/100/1024/MAX | PASS |
| MV2/S9 factor cero/prioridad | FactorNuloYRecuperacion, PrioridadErrores; oráculo | PASS |
| MV3/S10 dominio completo | RedondeoConFactor, ExtremosGenericos, MonotoniaConFactor, oráculo exacto | PASS |

15 casos anteriores migrados con factor TPI=60, incluyendo 100 fronteras y
10000 ventanas del oráculo por distancias original. Ocho casos nuevos: total
23 casos B03; monotonía adicional en 6150 puntos. Regresiones de otros bloques
sin cambios: 79 casos. Total C **102/102**.

Oráculo Python estándar con enteros arbitrarios: **11199 ternas** deterministas,
producto cartesiano, barrido y fronteras de redondeo/rango. Evalúa directamente
(2×pulsos×60000000000+D)//(2×D), distinto del algoritmo binario de producción.
Comprueba estado, valor y consistencia del registro usando el consumidor C.
8465 válidas, 2038 de rango, 333 de duración y 363 de factor cero;
9016 entradas válidas de tiempo/factor tienen numerador matemático superior a uint64_t.
Incluye MAX/MAX/MAX→14 y denominadores impares; sin generar código de referencia
desde las funciones bajo prueba. No hay simulación de tiempos/eléctricos.

## Comandos y resultados realmente ejecutados

OUT=/tmp/ed3-b03-r1-20261107-6hxxb4jm. Cada carpeta contiene comandos.json y logs completos fuera de Git.

| Configuración | Comandos principales | Resultado |
|---|---|---|
| Host Debug | cmake -S . -B "$OUT/host/build" -DTPI_BUILD_ARM=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug; build; ctest -V --output-on-failure | 8/8 CTest: 102 casos C + 11199 ternas |
| Sanitizadores Debug | Igual en san/build con -DTPI_HOST_SANITIZERS=ON; ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest | 8/8 sin hallazgos ASan/LeakSanitizer/UBSan; incluye consumidor del oráculo |
| ARM Debug | -DTPI_BUILD_ARM=ON -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"; build; ctest -V | 8/8; B00 conserva 14 casos y fixtures B01–B05 |
| ARM Release | Configuración anterior con -DCMAKE_BUILD_TYPE=Release; build; ctest -R '^b03\.' | 1/1 enlace B03, APIs/división64 retenidas |
| Claridad de unidades final | Rebuild host/san/ARM después de nombrar SEGUNDOS_POR_MINUTO; CTest -R '^b03\.' | Host/san 2/2; ARM Debug/Release 1/1; regresiones ajenas anteriores intactas |
| Aislamiento | cc -std=c11, avisos estrictos, sólo -IServicios; mismo consumidor del RED + Medicion_Velocidad.c; verificar_velocidad.py --programa consumidor | 11199 ternas correctas; sin incluir/enlazar configuración/dispositivo |
| Documentación | doxygen - con Doxyfile versionado, OUTPUT_DIRECTORY temporal; XML | Sin avisos; 21 funciones, dos USW B03 y configuración externa |
| Recursos | arm-none-eabi-gcc -O0 -ffreestanding -fstack-usage; size/nm/readelf/objdump; verificar_elf.py | Tabla siguiente, sin símbolos indefinidos en ELF |

Sanitizadores se ejecutaron fuera de ptrace/restricciones que impiden fugas, sin
desactivar controles. No hay umbral de cobertura configurado; no se midió cobertura.
Sin CI nuevo, instalaciones ni cambios de configuración sensible del MCU.

Durante verificación se corrigieron dos errores del comprobador temporal:
la primera lectura XML omitía grupos Doxygen (sólo encontraba main) y el primer
filtro de dependencia confundía el include guard TPI_MEDICION_VELOCIDAD_H con
la constante física. Se incluyeron grupos y se comprobó el identificador completo
de configuración; comprobaciones finales pasan sin suprimir documentación/aislamiento.
No fueron defectos del firmware ni avisos Doxygen.

## Recursos y límites de C

| Evidencia ARM | Resultado estático |
|---|---|
| Módulo -O0 | .text 828 bytes, .data/.bss 0; única dependencia __aeabi_uldivmod |
| Pila propia | Inicializar 16; Actualizar 224 bytes, estática |
| Cadena de división | 224+16+32=272 bytes internos como máximo; excluye llamador/IRQ |
| Fixture Debug | Flash 2520; SRAM 2068, incluida pila reservada 2048 |
| Fixture Release | Flash 2212; SRAM 2068, incluida pila reservada 2048 |
| ELF | Validador B00 PASS; sin indefinidos; GNU_STACK RW; lectura permanece de 8 bytes |

A=pulsos×1000000000<=4294967295000000000;
D=duración×factor<=18446744065119617025. Ambos caben en uint64_t.
Se preserva resto exacto al dividir A, se escala por seis pasos binarios sin formar
60×resto y se redondea una sola vez mediante resto>=D-resto. Sólo constantes
de conversión de unidades quedan en servicio. Resultado y cast se validan antes
de estrechar; sin flotantes, heap, recursión, globals mutables o espera.
La pila de libgcc se inspeccionó en desensamblado; no es medición de ejecución real.

## Cierre y limitaciones

Sin incidencias críticas/avisos funcionales pendientes del alcance. Tareas 11/11 al
completar commit. Se revisa diff/índice, YAML, enlaces, formato y hashes de contratos,
archivos, aplicación/dispositivos/bibliotecas y resto de bloques no afectados.
Informe B03 archivado sigue siendo evidencia del factor fijo histórico, no de la API nueva.
Esta revisión es candidata y requiere aceptación para promover el contrato.
No hay captura real, timeout, filtrado, engranajes, regulación/main ni validación física.
No se ejecutó firmware ARM, simulador o placa. No hay garantía de tiempo eléctrico.
Próxima acción: aceptación de B03-R1, luego sincronización/archivo si el usuario lo ordena.

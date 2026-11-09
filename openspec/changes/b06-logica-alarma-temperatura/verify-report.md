# Verificación B06 — 2026-11-09

Resultado: **PASS para el alcance lógico**, pendiente de aceptación. Base
dd80d582e880b4867fe50019b43a5c1b5908daf3. Commit local con asunto
`feat: evaluar alarma de temperatura configurable`; consultar Git para hash.
Sin push, promoción de contrato ni archivo B06.

## Implementación y coherencia

Procesamiento_EvaluarAlarmaTemperatura recibe const Temperatura_Lectura* y
umbral_centesimas_c int32_t de configuración explícito. Retorna enum completo:
ACTIVA/INACTIVA/SIN_DATOS/ERROR_FUENTE/ERROR_ARGUMENTO. No necesita contexto,
inicialización o salida adicional: cada retorno reemplaza la decisión del llamador.
Los valores de pruebas 2500/2534 no son configuración por defecto del TPI.

Sólo VALIDA compara temperatura>umbral; igualdad es INACTIVA. Ausencia/errores
se comprueban antes del valor y conservan su categoría; NULL/enum desconocido
son ERROR_ARGUMENTO. Comparación int32_t sin restar evita overflow en extremos.
No guarda lecturas/decisiones anteriores ni escribe entrada, modos, consignas o motor.
Nueva USW.2.1.2.7 documentada; cuerpos/tipos/firmas B01 anteriores idénticos.
Aplicación incluye contrato de servicio Temperatura.h, sin conocer LM35/ADC/pines.

Doxyfile ya cubría los archivos: no se cambia. Fixture B01 sólo agrega -IServicios
por el encabezado público nuevo; lógica y controles anteriores preservados.
Se agrega test/fixture B06 con API B01 y Temperatura, sin enlazar dispositivos o libc ARM.

## Matriz de aceptación — 8/8 escenarios

| Req./escenario | Prueba/evidencia ejecutada | Resultado |
|---|---|---|
| AT1/S1 umbrales | UmbralConfigurable: menor/igual/mayor en -5000/-1/0/2500/10000 | PASS |
| AT1/S2 extremos/configuración | ExtremosInt32, CambioUmbral; INT32_MIN/MAX y signo negativo | PASS |
| AT2/S3 disponibilidad/argumentos | CeroValidoVsAusencia, ErroresYArgumentos; NULL, enum99 y dato extremo inválido | PASS |
| AT2/S4 invalidación/recuperación | SecuenciaInvalidacionYRecuperacion, integración B05 | PASS |
| AT3/S5 independencia/inmutabilidad | EntradasInmutables, LecturasIndependientes; campos anteriores conservados | PASS |
| AT3/S6 servicio B05 | IntegracionServicioTemperatura, IntegracionErrorServicio; productor real genérico | PASS |
| AT3/S7 gestión de modos | ModoManualConAlarma, AutoEsperaConAlarma, AutoActivoConAlarma; contexto/salida y siguiente actualización B01 | PASS |
| AT4/S8 verificación/alcance | Suites, ELF, Doxygen, cuerpos B01/preservación y revisión de diff | PASS |

13 casos nuevos de C + 102 regresiones = **115 casos C**. CTest host incluye además
el oráculo B03-R1 de **11199 ternas**, reejecutado sin cambios. Sin tests omitidos.

## Pruebas realmente ejecutadas

OUT=/tmp/ed3-b06-20261109-baw2h0g0. Logs/comandos completos fuera del árbol versionado.

| Configuración | Comandos principales | Resultado |
|---|---|---|
| Consumidor primero | cc C11 estricto, test_alarma_temperatura.c + Procesamiento.c + Temperatura.c | Antes no compila por API ausente; después mismo archivo exacto pasa 13/13 |
| Host Debug | cmake -S . -B "$OUT/host/build" -DTPI_BUILD_ARM=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug; build; ctest -V --output-on-failure | 9/9 CTest; 115 C + 11199 ternas |
| Sanitizadores | Mismo flujo san/build con -DTPI_HOST_SANITIZERS=ON; ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest | 9/9, sin hallazgos ASan/LeakSanitizer/UBSan |
| ARM Debug | -DTPI_BUILD_ARM=ON -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"; build; ctest -V | 9/9, incluyendo B00 con 14 casos y fixtures B01–B06 |
| ARM Release | -DCMAKE_BUILD_TYPE=Release; build; ctest -R '^b06\.' -V | 1/1, API B06/B01/Temperatura retenida |
| Doxygen | Doxyfile existente, doxygen - con OUTPUT_DIRECTORY temporal; inspección XML | Sin avisos; 22 funciones/USW, incluida USW.2.1.2.7 |
| Consumidor puro | cc C11 estricto: consumidor + Procesamiento.c, sin Temperatura.c/dispositivos; ejecución | Compila/enlaza y decide ACTIVA sobre lectura genérica |
| Recursos | GCC ARM -O0 -ffunction-sections -fstack-usage; size/nm/readelf/objdump; sonda sizeof; verificar_elf.py | Datos siguientes |

Fuentes de pruebas anteriores y algoritmo del oráculo B03-R1 intactos.
Sanitizadores fuera de ptrace/restricciones de fugas, sin desactivar controles.
No hay umbral de cobertura configurado: no se midió cobertura ni se agregó CI.

Controles negativos temporales, fuera de Git: sustituir > por >= compila pero
falla S1; convertir SIN_DATOS en INACTIVA compila pero falla S3. Ambos fallos
esperados ejecutados y registrados en negativos/. La suite detecta esas regresiones.
No son fallos pendientes ni se modificó producción para ejecutar los controles.

## Recursos estáticos ARM y documentación

| Elemento | Medición |
|---|---|
| Nueva evaluación -O0 | 78 bytes de código, 16 bytes de pila propia; sin llamadas/auxiliares |
| Procesamiento completo -O0 | 826 bytes .text, .data/.bss 0; sin símbolos externos indefinidos |
| Tipos con toolchain actual | Enum de decisión 1 byte; Temperatura_Lectura 8 bytes |
| Fixture Debug | Flash 2048; SRAM 2108, incluida pila reservada de 2048 |
| Fixture Release | Flash 1536; SRAM 2112, incluida pila reservada de 2048 |
| Ambos ELF | Validador B00 PASS, APIs retenidas, sin indefinidos, GNU_STACK RW |

Pila propia excluye llamador/IRQ y no es medición física. Las diferencias del fixture
incluyen optimización/alineación de su integración, no nuevos globals de producción.
Un primer comprobador temporal suponía enum de 4 bytes y falló: una sonda sizeof
independiente y símbolos del ELF confirmaron 1 byte con este compilador ARM.
Se corrigió el comprobador para contrastar con la medida, sin cambiar ABI/flags
ni código de firmware. Tamaños son evidencia de esta compilación, no contrato portable.

Doxygen: siete USW de Procesamiento, seis Comunicacion, dos Medicion_Velocidad,
dos Mandos, dos Temperatura, dos LM35 y main = 22. Se conservan IDs .1–.6 de
Procesamiento; enum, unidades, retornos, errores, concurrencia y límites documentados.
Sin comentarios en cuerpos de funciones de producción ni dependencia instalable nueva.

## Cierre y límites

Sin incidencias funcionales críticas/avisos pendientes del alcance. Revisión:
diff/índice, YAML/enlaces, hash del consumidor RED/GREEN, seis cuerpos B01 y tipos
previos idénticos, fuentes/build ajenos e historial/contratos aceptados preservados.
Tareas 11/11 al completar commit. Documentación/contexto/mapa actualizados.

No hay ADC/DMA, actuación LED/buzzer/GPIO, parada del motor, histéresis, enclavamiento,
timeout, main integrado, prioridad temporal ni respuesta física a fallo de lectura.
ACTIVA/INACTIVA son decisión lógica disponible; los otros estados no ordenan actuadores.
Acceso coherente/serializado externo, sin detección de frescura o precisión eléctrica.
No se ejecutó firmware ARM, simulación o hardware ni se flasheó/instaló nada.
Próxima acción: aceptación del resultado B06; sólo luego sincronizar/archivar por orden.

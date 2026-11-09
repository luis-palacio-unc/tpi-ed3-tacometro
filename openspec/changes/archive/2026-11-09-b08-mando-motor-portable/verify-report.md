# Verificación B08 — 2026-11-09

**PASS para alcance lógico**. Alcance acordado, resultado pendiente de aceptación.
Base main: 501ab143ded9a59e4db39229733827b800213aab. Commit local autorizado;
asunto previsto `feat: aplicar mando de motor mediante salida portable`. Sin push.
Tareas 11/11 completas; revisión e índice preparados para commit local.
Candidato MM1–MM4/S1–S8; no se sincronizan contratos vigentes ni se archiva el cambio.

## Ejecución y entorno

Logs, comandos exactos y artefactos fuera de Git: /tmp/ed3-b08-20261109-qk7i0zj2.
GCC host 13.3.0, GCC ARM 13.2.1, CMake 3.28.3, Python 3.12.3, Doxygen 1.9.8.
C11 con -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes;
consumidores/fixture también -pedantic-errors, sin relajar controles.

| Verificación ejecutada | Resultado | Evidencia externa |
|---|---|---|
| Consumidores antes de API | RED esperado en ambas suites: Driver_Motor ausente | test_*.c.red.log, red.json |
| Mismos consumidores tras implementar | GREEN: 10 servicio + 5 integración | test_*.c.green-build/run.log |
| Host Debug, build y CTest | 13/13; 146 casos C y oráculo B03-R1 de 11199 ternas PASS | host-configure/build/ctest.log |
| ASan + LeakSanitizer + UBSan | 13/13 y mismas cantidades, sin hallazgos | san-ctest-proceso-normal.log |
| ARM Debug, firmware y CTest | 11/11; B00 conserva sus 14 casos | arm-configure/build/ctest.log |
| Fixture B08 Release -O3 | Enlace/ELF/símbolos/indefinidos/GNU_STACK PASS | arm-b08-release.log |
| Doxygen HTML/XML estricto | 25 funciones/USW; nuevas .2.1.8.1/.2 y typedef documentados; cero avisos | doxygen.log, doxygen-warnings.log, doxygen-audit.json |
| Saturación indebida (mutante temporal) | La suite compiló y falló como se esperaba al enviar 1001 | mutante-saturacion-build/run.log |
| Cero equivocado (mutante temporal) | La suite compiló y falló como se esperaba al solicitar cero | mutante-cero-equivocado-build/run.log |
| Servicio aislado y recursos | Compila sólo con Servicios/Configuracion/std, sin símbolos externos directos | servicio-dependencias.log, arm-servicio-*.log, extra.json |

Incidente resuelto: la primera corrida de sanitizadores dentro del entorno con
ptrace falló 13/13 porque LeakSanitizer no admite ese seguimiento. Se conserva
san-ctest.log y san.json con exit 8. Se repitió sólo CTest en proceso normal sobre
los mismos binarios; ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. Sin desactivar controles ni ocultar error.

Comandos reproducibles: usar R=raíz y O=directorio externo nuevo. Para host/san,
`cmake -S "$R" -B "$O/host" -DTPI_BUILD_ARM=OFF -DCMAKE_BUILD_TYPE=Debug`,
`cmake --build "$O/host" --parallel 2`, `ctest --test-dir "$O/host" --verbose`;
san usa carpeta propia y añade -DTPI_HOST_SANITIZERS=ON con opciones anteriores.
ARM configura -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Debug y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake", luego build/CTest.
Release ejecuta cmake -P cmake/verificar_enlace_b08.cmake con SOURCE_DIR,
OUTPUT_DIR externo, TOOL_PREFIX, PYTHON y BUILD_TYPE=Release. Doxygen usa configuración
vigente con sólo OUTPUT_DIRECTORY/WARN_LOGFILE externos. Argumentos completos en logs.

## Matriz de cumplimiento

| Requisito / escenario | Prueba realmente ejecutada | Resultado |
|---|---|---|
| MM1/S1 dominio y repetición | RangoCompleto: 65536 valores uint16_t (1001 válidos, 64535 rechazados); RepeticionesSinCache | PASS host/san, valor exacto y una llamada por válido |
| MM1/S2 cero explícito | SolicitudDeCero, ErrorCeroYRecuperacion | PASS host/san, cero repetido y fallo observable |
| MM2/S3 validación/prioridad | InterfazInvalidaYPrioridad, RangoCompleto, RechazoNoEsParada | PASS host/san, argumento antes de rango, sin envío ni saturación |
| MM2/S4 fallo/recuperación | ErrorSalidaYRecuperacion, ErrorCeroYRecuperacion, FalloDelProveedorObservable | PASS host/san, un intento por solicitud, sin error retenido |
| MM2/S5 configuración/contextos | ContextoNuloPermitido, InstanciasIndependientes, InterfazInmutable | PASS host/san, dos funciones/contextos distintos y const preservado |
| MM3/S6 manual/entrada automática | ManualDesdeEscalas, EntradaAutoSolicitaCero, FalloDelProveedorObservable | PASS host/san, B04→B01→mando/cero, espera con consigna antigua |
| MM3/S7 valores no consumibles | AutomaticoNoUsaRPMComoPWM, MandoNoValidoNoSeConsume | PASS host/san; no consumo de B04 no válido ni emisión de RPM como PWM |
| MM4/S8 evidencia | CTest host/san, ARM, Doxygen, mutantes, recursos y revisión de preservación/diff | PASS en sus modalidades, sin ejecución ARM/física |

10 casos del servicio + 5 integración + 131 anteriores = 146 C. Los 13 CTest host
incluyen 12 ejecutables C y un oráculo Python. La integración es adaptación de prueba,
no un ensamblado de producción; no acredita que main real accione motor.

## Recursos, coherencia y límites

Servicio ARM -O0: .text 102 bytes, .data/.bss 0. GCC informa 16 bytes propios por
función; ruta SolicitarCero→Aplicar suma 32 antes del proveedor. Excluye pila del
llamador, proveedor e interrupciones. Interfaz 8 bytes, enum 1 con este toolchain;
son mediciones de ABI actual, no contrato portátil. Sin heap/globals mutables,
aritmética, divisiones, esperas, caché ni llamadas externas directas en el objeto.
Fixture B08+B04/B01 Debug: Flash 3120, SRAM local 2112 bytes; Release: 2420/2112.
Incluyen pila reservada 2048; no miden profundidad total. Libgcc ya existente resuelve
aritmética de Mandos; Driver_Motor no la necesita. Ambas USW/proveedor y API B04/B01
retenidos en ELF con --gc-sections; sin indefinidos/libc, GNU_STACK RW.

Diseño respetado: uint16_t, límite común, interfaz externa const, prioridad de errores,
rechazo, SolicitarCero delega Aplicar, retorno completo y dos USW. El llamador valida
fuentes más amplias/con signo antes de convertir; mantiene vida/serialización y consume
retorno. Proveedores deben retornar; aceptación/fallo no determina estado eléctrico.
Snapshot/hash y diff comprueban preservación de aplicación, otros servicios, dispositivos,
Drivers, pruebas previas, linker/startup, contratos vigentes y archivos históricos.
Nuevas pruebas/fixture/candidato y documentación son el alcance; ninguna dependencia nueva.

Sin incidencias funcionales críticas pendientes. No se ejecutó firmware ARM, simulador
ni placa. PWM/pines/frecuencia/reloj/etapa de potencia, control automático, integración
de main y validación eléctrica/mecánica/temporal quedan pendientes. Sin flasheo,
instalación o cambio sensible MCU. Diagramas para el informe siguen diferidos según CONTEXTO.
Siguiente acción: revisar resultado B08; sincronizar/archivar sólo tras aceptación y
por orden. No publicar este commit sin autorización independiente.

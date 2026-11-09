# Verificación B07 — 2026-11-09

Resultado: **PASS para alcance lógico**, pendiente de aceptación. Base publicada
eb713e04cebd93068c08161cd67af9ede31d1388. Commit local con asunto
`feat: coordinar salidas de alarma mediante interfaz portable`; consultar Git para hash.
Sin push, promoción de contrato o archivo B07.

## Implementación y coherencia

Servicios/Driver_Alarmas.h/.c contiene Driver_Alarmas_Aplicar (USW.2.1.6.1),
interfaz const con dos funciones bool(void*,bool) y contextos independientes.
No requiere init, heap, registro global o memoria de órdenes/resultados.
Interfaz NULL o función ausente produce ERROR_ARGUMENTO sin invocar ninguna salida.
Los contextos NULL se permiten si la función provista los admite; son opacos al servicio.

Se invoca LED y después buzzer en expresiones separadas, una vez por canal,
incluso si LED informa fallo. Dos aceptaciones producen ACEPTADA; fallos de LED,
buzzer o ambos producen su error explícito. No hay reintentos, rollback o éxito total
ante fallo parcial. La llamada siguiente recupera si sus proveedores aceptan.
ACEPTADA significa aceptación declarada de esta solicitud, no confirmación eléctrica.

Adaptación B06 sólo en pruebas/fixture: ACTIVA/INACTIVA generan true/false;
SIN_DATOS/errores no emiten órdenes ni se interpretan como apagado. El adaptador
devuelve si hubo emisión; resultado del servicio sólo se consume si hubo llamada.
El diagnóstico B06 se conserva. No hay dependencia Servicios→Aplicacion,
temperatura, umbral, MCU, GPIO o dispositivos concretos en el servicio.
Aplicacion/Procesamiento y servicio Temperatura, main y dispositivos quedan intactos.

## Matriz de aceptación — 8/8 escenarios

| Req./escenario | Pruebas/evidencia ejecutadas | Resultado |
|---|---|---|
| SA1/S1 solicitudes | ActivarAmbas, DesactivarAmbas, RepeticionSinCache | PASS |
| SA1/S2 orden/instancias | OrdenDeCanales, InstanciasIndependientes; LED→buzzer y contextos distintos | PASS |
| SA2/S3 interfaz inválida | InterfazInvalidaSinLlamadas; NULL/funciones ausentes sin efectos | PASS |
| SA2/S4 fallos/recuperación | FalloLED, FalloBuzzer, FallanAmbos, RecuperacionSinEstadoRetenido; integración ErrorDeSalidaYRecuperacion | PASS |
| SA2/S5 contextos/const | ContextosNulosPermitidos, InterfazInmutable | PASS |
| SA3/S6 integración evaluable | DecisionesEvaluables con productor B05 y evaluación B06; servicio compilado aisladamente | PASS |
| SA3/S7 no evaluable | EstadosNoEvaluables, PerdidaDeLecturaNoEmiteApagado; sin órdenes/nuevo éxito y diagnóstico retenido | PASS |
| SA4/S8 verificación | CTest, sanitizadores, ELF Debug/Release, Doxygen, recursos y preservación/diff | PASS |

12 casos C del servicio + cuatro de integración + 115 regresiones = **131 casos C**.
CTest host ejecuta además el oráculo B03-R1 de **11199 ternas**, sin cambios.
Los proveedores emulados registran solicitudes/aceptaciones; no representan niveles eléctricos.

## Comandos y resultados realmente ejecutados

OUT=/tmp/ed3-b07-20261109-v_30cvtq. Comandos.json/logs por configuración fuera del árbol versionado.

| Configuración | Comandos principales | Resultado |
|---|---|---|
| Consumidores primero | cc C11 estricto para unidad/integración contra API previa, luego mismo código | RED esperado por API ausente; GREEN 12/12 y 4/4, hashes idénticos |
| Host Debug | cmake -S . -B "$OUT/host/build" -DTPI_BUILD_ARM=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug; build; ctest -V --output-on-failure | 11/11 CTest: 131 C + 11199 ternas |
| Sanitizadores | Igual en san/build con -DTPI_HOST_SANITIZERS=ON; ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest | 11/11, sin hallazgos ASan/LeakSanitizer/UBSan |
| ARM Debug | -DTPI_BUILD_ARM=ON -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"; build; ctest -V | 10/10; B00 conserva 14 casos y fixtures B01–B07 |
| ARM Release | -DCMAKE_BUILD_TYPE=Release; build; ctest -R '^b07\.' -V | 1/1: API B07/B06/B05 y proveedor retenidos |
| Servicio aislado | cc C11 estricto, sólo -IServicios, test_driver_alarmas.c + Driver_Alarmas.c; target CMake igualmente aislado | 12/12; sin rutas/fuentes de aplicación, configuración o dispositivos |
| Doxygen | Doxyfile ampliado, doxygen - con OUTPUT_DIRECTORY temporal; XML | Sin avisos; 23 funciones/USW, nueva USW.2.1.6.1 y typedef de proveedor documentado |
| Recursos | GCC ARM -O0 -ffreestanding -fstack-usage; size/nm/readelf/objdump, sizeof por sonda y verificar_elf.py | Tabla siguiente |

Controles negativos temporales: buzzer siempre recibe true, la suite falla en
S1 DesactivarAmbas; retorno temprano tras fallo LED, falla S4 FalloLED por omitir
buzzer. Ambos compilan y ambos fallos esperados se ejecutaron/registraron en negativos/.
No se alteró producción ni se dejaron fallos funcionales pendientes.
Sanitizadores ejecutados fuera de ptrace/restricciones de fugas, sin suprimir controles.
No hay cobertura mínima configurada; no se midió cobertura ni se agregó CI/dependencias.

## Recursos estáticos y documentación

| Elemento ARM actual | Medición |
|---|---|
| Driver_Alarmas completo -O0 | .text 144 bytes, .data/.bss 0; sin símbolos externos indefinidos |
| Pila propia Aplicar | 24 bytes, estática; excluye pila de proveedores/llamador/IRQ |
| Tipos por sizeof independiente | Interfaz 16 bytes, enum de resultado 1; coincidencia con símbolos del ELF |
| Fixture Debug | Flash 1464; SRAM 2084, incluida pila reservada 2048 |
| Fixture Release | Flash 1176; SRAM 2084, incluida pila reservada 2048 |
| Ambos ELF | Verificador B00 PASS, APIs/proveedor retenidos, sin indefinidos, GNU_STACK RW |

No se impone tamaño portable de enums/punteros. Duración y pila totales dependen
de las funciones de salida: sólo se acreditan dos invocaciones seriales y marco
propio, sin espera/reintento del servicio. No es medición física de tiempos/pila.
Doxygen: 22 funciones previas conservadas + Driver_Alarmas_Aplicar = 23.
ESW.2.1.6/USW.2.1.6.1 y contratos de función/contexto, retorno, validación,
serialización, semántica lógica y errores documentados. Sin comentarios en cuerpos
de funciones de producción ni cambios a controles Doxygen.

## Cierre y límites

Sin incidencias críticas/avisos funcionales pendientes del alcance. Revisar diff/índice,
YAML/enlaces, presupuesto documental y hashes: aplicación, otros servicios, dispositivos,
bibliotecas, plataforma, pruebas anteriores y contratos/archivos aceptados preservados.
Ningún contrato vigente cambia; salidas-alarma sigue candidato. Tareas 11/11 al completar
commit. Estado/contexto/mapa actualizados con evidencia real.

No hay implementación GPIO/LED/Buzzer físicos, polaridad/pines, main, prioridad temporal,
patrón físico ante pérdida de lectura, histéresis, enclavamiento, reintentos o timeout.
La adaptación mostrada está en pruebas; producción no incorpora aún el ensamblado real.
No se acciona motor ni se cambia umbral/decisión B06. Interfaz/contextos deben mantenerse
válidos/estables y serializados; proveedor debe retornar según contrato.
No se ejecutó firmware ARM, simulador o placa. Sin flasheo/instalación/cambios MCU.
Próxima acción: aceptación del resultado B07; sólo después sincronizar/archivar por orden.

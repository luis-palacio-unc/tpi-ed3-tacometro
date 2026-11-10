# Verificación B07-R1 — salidas sin contexto

2026-11-10. Alcance/criterio acordados; implementación verificada, resultado pendiente
de aceptación. Base bae59e1, main; commit normal local autorizado, sin push.

## Implementación y coherencia

Driver_Alarmas_EscribirSalida es void(bool); Driver_Alarmas_Salidas contiene sólo
escribir_led/escribir_buzzer. Aplicar valida la interfaz completa antes de accionar
LED→buzzer una vez por canal y retorna ACEPTADA. Interfaz NULL/función ausente:
ERROR_ARGUMENTO sin accionar ninguna salida; llamada válida posterior recupera.
Eliminados contextos y ERROR_LED/ERROR_BUZZER/ERROR_AMBOS, sin adaptadores/casts.
ACEPTADA significa despacho, sin diagnóstico físico. Doxygen mantiene ESW.2.1.6 y
USW.2.1.6.1; parámetro/unidad lógica, resultados, precondiciones y concurrencia.

Migrados todos los consumidores actuales: pruebas de servicio/térmicas y fixture
ARM. Nueva composición host usa Driver_Alarmas y LED_Write/Buzzer_Write reales
contra doble API GPIO, sin modificar Dispositivos. B06 conserva decisión y ausencia/
error sin órdenes. No dependencia del servicio hacia Aplicacion/Dispositivos/Drivers.
Diseño seguido sin desviaciones; sin init, estado mutable, heap, esperas o reintentos.

## Ejecución real

Logs/comandos fuera del árbol: `/tmp/ed3-b07r1-salidas-4pkq_eul`.
C11 estricto -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes;
pruebas/extras también -pedantic-errors. Sin controles desactivados ni instalaciones.

| Control | Resultado | Evidencia externa |
|---|---|---|
| RED antes del cambio de servicio |Incompatibilidad del callback anterior detectada |red-api.log |
| GREEN dirigido |3/3 suites;8 servicio,4 térmicos,8 GPIO |green-*.log |
| Host completo |18/18,183 casos C y oráculo Python; cero omisiones |host-*.log |
| ASan/LSan/UBSan |18/18, detect_leaks=1, sin hallazgos fuera de ptrace |san-*.log |
| ARM Debug |Build y13/13 controles de imagen/enlace |arm-*.log |
| Fixture B07 ARM Release |Enlace/imagen/símbolos válidos sin libc ni MCU ejecutado |b07-arm-release-fixture.log |
| Servicio aislado ARM-O0/O2 |Compila sólo con Servicios; sin símbolos externos directos |servicio-*.log |
| Header aislado C11/C++17 |Sin CMSIS/dispositivos, correcto |header-*.log |
| Doxygen HTML/XML estricto |38 funciones únicas sin avisos; USW6.1 y enum de dos estados |doxygen.log, extra-results.json |
| Negativos externos |Detectan orden invertido, omisión de buzzer y accionamiento antes de validar |*-run.log |
| Preservación |245 archivos protegidos intactos;90 Drivers originales a21cc1e |before.json, preservation.json, diff |

Casos de servicio: ActivarAmbas, DesactivarAmbas, RepeticionSinCache,
OrdenDeCanales, InterfazInvalidaSinLlamadas, RecuperacionTrasArgumentoInvalido,
InterfacesIndependientes e InterfazInmutable. Interfaz inválida cubre NULL y tres
combinaciones de funciones ausentes, con true y false. Registros de llamadas
comprueban cantidad, orden y valores, no sólo códigos de retorno.
Se retiraron cuatro casos que exigían contexto/errores de proveedor eliminados,
se migraron recuperación/interfaces y se añadió composición directa:186→183 casos.
Regresiones ejecutadas: velocidad11199 ternas, codec5096 pares, selector4096 muestras/
400 vueltas. Los siete casos GPIO anteriores se conservan y añade composiciónS9.

## Matriz de cumplimiento

| Requirement/escenario | Prueba ejecutada | Resultado |
|---|---|---|
| SA1/S1 |ActivarAmbas/DesactivarAmbas/RepeticionSinCache/OrdenDeCanales |Conforme |
| SA1/S2 |InterfacesIndependientes, dos pares de funciones distintos |Conforme |
| SA2/S3 |InterfazInvalidaSinLlamadas,8 solicitudes sin efectos |Conforme |
| SA2/S4 |RecuperacionTrasArgumentoInvalido y prueba térmica de argumento |Conforme |
| SA2/S5 |InterfazInmutable y validación sin modificar configuración |Conforme |
| SA3/S6 |DecisionesEvaluables, argumento inválido conserva decisión |Conforme |
| SA3/S7 |EstadosNoEvaluables/PerdidaDeLecturaNoEmiteApagado |Conforme |
| SA3/S9 |ComposicionAlarmasSinAdaptadores,4 patrones×4 solicitudes |Conforme |
| SA4/S8 |Host/san/ARM/aislamiento/Doxygen/hashes/diff |Conforme por modalidad |

9/9 escenarios conformes con pruebas ejecutadas; umbral de cobertura no configurado.
El doble de API GPIO no ejecuta biblioteca, CPU ni fenómenos eléctricos.

## Recursos, límites y cierre

Servicio ARM: .text68 bytes-O0/40-O2, .data/.bss0; marco propio16 bytes en ambos.
Sin funciones auxiliares nuevas; pila/tiempo total dependen de las funciones de
salida y de la integración/IRQ, no acreditados por estos marcos. Dos punteros de
función por interfaz; se retiran dos punteros de contexto, sin retener configuración.
Fixture Release: huella FLASH1160 bytes; SRAM local2084 incluyendo pila reservada2048.
Es una imagen de prueba con salidas emuladas, no consumo final del firmware.

Main, B06/B12, plataforma, restantes servicios, biblioteca y contratos/archivos
aceptados permanecen intactos. Sincronizar especificaciones y archivar sólo al aceptar
el resultado; el contrato salidas-alarma vigente aún describe B07 anterior.
Inicialización GPIO/mux/dirección/polarización/FIOMASK, reloj/enlace global/main y
ensayos físicos siguen diferidos. No ejecución ARM/placa, simulación MCU ni flasheo.

Reproducción: cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug;
cmake --build "$O/host" --parallel 4; ctest --test-dir "$O/host" --verbose.
San: -DTPI_HOST_SANITIZERS=ON con ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM: -DTPI_BUILD_ARM=ON y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Comandos exactos/exits en
host/san/arm/extras-commands.json. Doxygen vigente con OUTPUT_DIRECTORY externo.

Veredicto: PASS para el alcance acordado. Sin fallos críticos/desviaciones.
12/12 tareas de implementación/registro completas; aceptación/sincronización/archivo
todavía pendientes. Próxima acción: revisión del resultado B07-R1 por el usuario.

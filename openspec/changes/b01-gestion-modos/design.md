# Diseño B01

## Decisiones

| Elección | Alternativa | Motivo |
|---|---|---|
| Dos funciones públicas con contexto explícito | Estado global | Pruebas aisladas, sin heap y sin interferencia entre instancias |
| Entradas/salidas por estructuras C | Leer servicios directamente | Separar regla de selección de pines, UART y PWM aún pendientes |
| Orden uint64_t sin rollover, no tiempo en ms | Comparación modular de ticks de 32 bits | Evitar ambigüedad de paquetes muy antiguos y límites de media vuelta |
| Consigna con orden asignado al recibir | Marcar al retirar de cola | Rechazar datos en cola previos al cambio de modo |
| Mando manual entero 0..1000 por mil | Float o ADC crudo | Unidades estables sin referencia/pines/resolución ADC |
| Una fuente enum por salida | Dos habilitaciones booleanas | Representar exclusión sin combinaciones simultáneas |
| Cambio de modo antes de consigna en la misma llamada | Aplicarla inmediatamente | Conservar entrada a automático con salida cero |

Orden es una secuencia lógica externa monotónica; no implica reloj/ISR configurado.
Al integrar recepción, el productor deberá registrar orden al recibir y preservar esa
marca en cola. Las llamadas a Procesamiento y la captura deberán serializarse; B01 no
es thread-safe y no define cómo bloquear interrupciones. Reiniciar orden requiere
inicializar contexto y purgar colas; nunca reutilizar eventos de una sesión anterior.

## Contrato público

`Procesamiento_Inicializar(contexto, modo, orden)` devuelve bool. Inicializa cada campo;
modo inválido deja espera e inhibe automático hasta entrada válida. NULL devuelve false.
`Procesamiento_Actualizar(contexto, entradas, salida)` devuelve bool. Ninguna función
accede a servicios o periféricos. El integrador debe aplicar fuente CERO como PWM cero;
AUTOMATICA sólo habilita al futuro regulador, no es un ciclo PWM calculado en B01.

Entradas: modo lógico, mando uint16_t por mil, orden uint64_t actual y puntero opcional
const a consigna {rpm uint32_t, orden_recepcion uint64_t, valida bool}.
Salida: estado, fuente, mando_manual_por_mil y consigna_rpm. Campos ajenos a la fuente
quedan en cero. El contexto se aloja estáticamente o en pila e incluye estado, órdenes
de actualización/entrada/última consigna, RPM y marca de recuperación; no modificar sus
campos directamente ni consultar antes de inicializar.

Consigna elegible sólo si válida y entrada_auto < orden_rx <= orden_actual,
además de orden_rx > última_aceptada. Una consigna inválida no sustituye la vigente.
RPM 0..185 inclusive se aceptan si el servicio las marca válidas. El máximo 185
fue aclarado por el usuario tras la primera implementación B01; Procesamiento lo
comprueba aunque valida sea true. No saturar ni limitar la medición real de velocidad. Orden agotado no se incrementa internamente; un rollover se
rechaza como regresión, con salida cero.

Errores de modo/orden/mando manual o punteros: limpiar salida disponible, invalidar
consigna y preservar el último orden confiable (o avanzar al actual no regresivo).
La siguiente actualización automática crea nueva barrera incluso si ya estaba en espera.
Mando fuera de rango en automático se ignora. Acceder a contexto no inicializado,
memoria inválida o estructuras solapadas viola precondiciones del llamador.

## Archivos y USW

Procesamiento.h/.c: ESW.2.1.2. USW .1 Inicializar, .2 Actualizar, .3 ModoValido,
.4 EstadoValido, .5 Invalidar y .6 LimpiarSalida. Contratos completos en header;
helpers documentados en .c; ningún comentario dentro de cuerpos de producción.
CMakeLists/CMakePresets: CTest host y opción de sanitizadores sólo para test host.
tests/host/test_procesamiento.c: casos de contrato que ejecutan C real.
cmake/verificar_enlace_b01.cmake: fixture ARM que fuerza referencia a API, pues main
no se integra aún y --gc-sections descarta la lógica no usada de la imagen mínima.
docs/doxygen/Doxyfile: entrada limitada a main/Procesamiento; advertencias como errores,
HTML/XML de validación; ampliar al desarrollar módulos. No es aún documentación final.

## Verificación y riesgos

Host normal y con AddressSanitizer/UBSan; fronteras, errores y transiciones S1–S8.
ARM Debug/Release y fixture de enlace de la API sin libc; B00 sin regresiones.
La comprobación B00 de objetos host construirá explícitamente target firmware,
para no contar objetos de nuevos tests. No se elimina ninguna prueba B00.
Doxygen 1.9.8 existente: ejecución estricta sin instalar dependencias y revisión XML.
Sin aceptación física. Revisión de diff, memoria y preservación del archivo B00.
No hay preguntas que bloqueen este alcance; el orden global de recepción se integrará
cuando se desarrollen comunicación y lazo principal.

# Diseño B10 — Selector lógico estable

## Decisiones

Ampliación de MA3 acordada; candidato hasta aceptación del resultado. Sin discrepancias
con B04/B01/B08. Estabilidad significa observación en muestras; no detecta rebotes
entre llamadas, ausencia de llamadas o múltiples vueltas omitidas del contador.

| Opción | Compromiso | Decisión/motivo |
|---|---|---|
| GPIO/polaridad / modo lógico | Hardware sin fijar y portabilidad requerida | Enum propio MANUAL/AUTOMATICO; adaptación externa, NINGUNO sólo salida inválida |
| Espera/timer interno / timestamp recibido | Bloquea y ata el servicio al MCU | uint32_t ms y resta modular, sin esperas o reloj propio |
| Modo inicial supuesto / disponibilidad | Podría habilitar una fuente sin lectura estable | SIN_DATOS/NINGUNO hasta confirmar, incluso tras reinicio |
| Contexto y salida separados / registro único | Duplicaría memoria y ciclo de vida sin necesidad | Mandos_Selector contiene modo/estado públicos y metadatos internos |
| Borrar modo durante candidato / retener confirmado | Rebotes no deben producir cambios espurios en B01 | Retener válido hasta confirmar otro; ante fallo invalidar ambos |
| Bloquear recuperación temporal / nueva época | Tras reiniciar reloj no existe anclaje confiable | ERROR_TIEMPO descarta anclaje; próxima válida recalifica desde cero |

## API y estados

Mandos.h/.c, ESW.2.1.5, sin cambiar tipos/constantes/cuerpos B04:
- Mandos_ModoSelector: MODO_NINGUNO, MODO_MANUAL, MODO_AUTOMATICO (prefijo MANDOS_SELECTOR_).
- Mandos_FuenteSelector: FUENTE_SIN_DATOS, FUENTE_VALIDA, FUENTE_ERROR (mismo prefijo).
- Mandos_EstadoSelector: SIN_DATOS, VALIDO, ERROR_ARGUMENTO, ERROR_CONFIGURACION,
  ERROR_FUENTE, ERROR_MODO, ERROR_TIEMPO (prefijo MANDOS_SELECTOR_).
- Mandos_Selector: intervalo_estable_ms, ultimo_tiempo_ms, inicio_candidato_ms uint32_t;
  modo/candidato, estado y bool tiene_tiempo. Sólo leer modo/estado; otros campos internos.
- MANDOS_SELECTOR_INTERVALO_MAX_MS = (uint32_t)INT32_MAX, sin default TPI.
- bool Mandos_InicializarSelector(Mandos_Selector*, uint32_t intervalo), USW.2.1.5.3.
  NULL false; cualquier contexto escribible queda completamente inicializado: intervalo
  recibido, SIN_DATOS o ERROR_CONFIGURACION, NINGUNO y sin anclaje/candidato. Excesivo
  devuelve false y actualizaciones siguen rechazadas hasta reiniciar con intervalo válido.
- Mandos_ActualizarSelector(Mandos_Selector*, Mandos_ModoSelector, Mandos_FuenteSelector,
  uint32_t tiempo_ms), USW.2.1.5.4; retorna estado completo, también selector->estado
  salvo NULL. Consumir selector->modo sólo con VALIDO; no es un nivel eléctrico.
- InvalidarSelector privado, USW.2.1.5.5: limpia modo/candidato/inicio y asigna estado;
  preserva configuración/anclaje, retorna estado. Lo usan inicialización y errores.

## Algoritmo y recuperación

Actualizar exige contexto inicializado/vivo y serialización externa. Validar NULL,
configuración, enum de fuente; luego delta uint32_t desde última llamada aceptada.
Delta>MAX informa ERROR_TIEMPO y borra tiene_tiempo/ultimo; permite nueva época.
Otro delta, incluido cero, registra ahora y tiene_tiempo=true. Fuentes SIN_DATOS/ERROR
invalidan antes de considerar modo; modo no disponible se ignora. Modo desconocido
con fuente válida invalida. Esos fallos conservan último tiempo aceptado. Fuente
desconocida invalida sin registrar tiempo nuevo. No restaurar confirmación anterior.

Con fuente/modo válidos, si cambia candidato guardar modo/ahora. Si ahora-inicio
modular>=intervalo, confirmar. Retornar VALIDO si existe modo confirmado, SIN_DATOS
en otro caso. Intervalo<=MAX y avance<2^31 aseguran evaluar candidato antes de perder
una vuelta completa; confirmado ya no depende de edad de su candidato original.
Sin sumas absolutas, restas con signo, heap, globals mutables, bucles o bloqueos propios.
Muestreo/captura coherentes externos; const/uint32_t no resuelven concurrencia ISR/lazo.

## Composición, archivos y verificación

Aplicación futura adapta modo confirmado a B01; pruebas solamente en este bloque.
B01 recibe orden de eventos uint64_t independiente de ms/rollover. Sólo transición
manual→auto establece barrera/cero; rebotes en auto activo no invalidan consigna.
Indisponibilidad no llama B01/B08 desde adaptador; no define retención física/parada,
ni reentrada de aplicación tras fallo. Esa integración/política sigue pendiente.

Mandos.h/.c agrega tres USW; tests/host/test_selector.c cubre fronteras/errores y
oráculo por historial de muestras/tiempo absoluto uint64_t, independiente del estado
incremental de producción. test_integracion_selector.c conecta B01/B08, sin cambiar
Procesamiento/Driver_Motor. CMakeLists.txt y cmake/verificar_enlace_b10.cmake agregan
targets/fixture; Doxyfile existente ya incluye Mandos. README/docs/agente/openspec
registran matriz/evidencia, sin modificar contratos vigentes/archivos aceptados.

Consumidores RED por API ausente, luego GREEN. Negativos temporales: no reiniciar
ventana al cambiar candidato y confirmar después del límite en vez de en el límite.
Ejecutar host/san (sin ptrace, fugas habilitadas), regresiones, ARM Debug y fixture
B10 Release, Doxygen estricto; medir código/pila/ABI sin prometer tamaño portable.
Sin instalación/migración ni preguntas bloqueantes. GPIO/timer real/main, hardware y
diagramas finales pendientes; commit local autorizado sin push.

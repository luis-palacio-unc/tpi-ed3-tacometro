# Diseño B11 — TIMER0 continuo y Tiempo

## Decisiones y flujo

| Opción | Elección y motivo |
|---|---|
| ISR/contador software o TC | TIMER0 exclusivo, libre. Leer TC conserva cuenta sin depender de consultas/interrupciones. |
| Registros en servicios | Sólo Base_Tiempos_LPC1769 conoce CMSIS. Tiempo consume proveedor reemplazable según B08. |
| Cambiar unidades con resolución | Período entero ms, normalización TC*período modulo 2^32 en dispositivo; servicios siempre ms. |
| Globales de aplicación/servicio | Ninguno nuevo. Dispositivo singleton contiene sólo período y estado, sin contador software. |

Prueba/fixture → Tiempo_Leer/Transcurrido → callback dispositivo → Base_Tiempos_Leer
→ puerto LeerTC → TC. Main sigue mínimo; composición real de arranque posterior.

## Contratos

Dispositivo: Inicializar(pclk_hz,periodo_ms), Leer(lectura*), Detener(); retornan
SIN_DATOS=0, ACTIVA, DETENIDA, ERROR_ARGUMENTO, ERROR_RELOJ, ERROR_PERIODO,
ERROR_PRESCALER. Lectura: uint32_t milisegundos y estado. ProveerMilisegundos(void*,
uint32_t*) retorna bool, ignora contexto opaco (admite NULL), true sólo ACTIVA;
salida escribible se limpia antes de intentar fuente. Dispositivo no incluye servicio.

Prioridad init: pclk 1..120000000, período 1..INT32_MAX, producto exacto/representable.
uint64_t producto=pclk*período cabe bajo 2^58; exigir producto%1000=0,
1<=producto/1000<=2^32, convertir N-1 a uint32_t. uint64_t es sólo cálculo acotado
de configuración; lectura usa producto uint32_t modular. No saturar marcas.
Inicializar siempre prepara/apaga/resetea timer, incluso error; período privado cero
si inválido. Válido inicia desde cero. Detener sólo ACTIVA, preserva contador/otros
estados; Leer SIN_DATOS/error no consulta TC, DETENIDA lee TC detenido. NULL no altera.
Metadatos serializados desde lazo privilegiado, sin ISR ni máscara global de IRQ.
TC es lectura alineada de 32 bits; no acceso concurrente a configuración.

Servicio: Tiempo_Fuente={bool (*leer)(void*,uint32_t*),void* contexto}; Tiempo_Lectura
={uint32_t milisegundos,Tiempo_Estado estado}. Estados NO_DISPONIBLE=0, VALIDO,
ERROR_ARGUMENTO, ERROR_INTERVALO. Leer(fuente*,lectura*) prioriza destino y después
interfaz, limpia salida y llama proveedor una vez con variable local. false ignora
valor y publica NO_DISPONIBLE/cero. Transcurrido(fuente*,inicio_ms,intervalo*) usa
Leer una vez, después delta uint32_t; >INT32_MAX produce ERROR_INTERVALO/cero.
Fuente falla antes de comprobar rango. Contextos/interfaz/destinos vivos, estables,
serializados/no solapados; proveedor debe retornar. Servicio sin estado propio/init.

## Puerto y recursos

Cuatro operaciones privadas: Preparar, Iniciar(PR), LeerTC y Detener. Preparar:
PCONP|bit1 conservando otros → TCR=0 → deshabilitar/limpiar NVIC TIMER0 → TCR=2
→ TC=PC=0 → CTCR=MCR=CCR=EMR=0 → IR=0x3f → DSB. No leer-modificar-escribir IR.
Iniciar escribe PR y TCR=1/DSB. Detener escribe TCR=0/DSB. LeerTC sólo lee TC.
No tocar MRx (MCR deshabilita acciones), otros timers, pines, PCLKSEL, PLL o SysTick.
No reutilizar helpers heredados que configuren relojes. Preservar Drivers/licencias.
La cuenta hardware depende de PCLK activo; no prometer suspensión/debug/clock gating.

## Archivos/USW

- Rehacer cuatro Base_Tiempos.*; públicas .17.1/.2/.3 mantienen rol; retirar .17.4–.8
  de candidato SysTick, proveedor nuevo .17.9 y puerto .17.10–.13. Ocho funciones.
- Nuevos Servicios/Tiempo.h/.c: extensión local ESW.2.1.18, USW .18.1/.2.
- Configuracion/Temporizacion.h: default TPI_BASE_TEMPORAL_MS=1, override de build
  positivo <=INT32_MAX; no incluir configuración en servicio genérico.
- Sustituir tests/modelos/fixture/verificador B11-SysTick por TC/Tiempo; CMake/Doxyfile
  ampliados, contexto actualizado. No modificar contratos vigentes/archivos aceptados.

## Verificación y transición

Modelo CMSIS host compila backend real y avanza PCLK/PC/TC según PR, sin ISR/API;
oráculo por tiempo absoluto y bases 1/2/3/5, límites PR=0/UINT32_MAX y errores.
Prueba del wrapper compila sólo con Servicios y proveedores falsos. Integración
incluye B10 sin cambiarlo. Sanitizadores con fugas fuera de ptrace. ARM Debug/Release
retiene APIs, verifica handlers SysTick/TIMER0 por defecto, lectura TC/disassembly,
memoria/pila/indefinidos y Doxygen estricto (42 funciones previstas).
Negativos: PR fuera de uno y consulta que reinicia TC deben fallar en pruebas.

Snapshot preserva planificación local ya existente y fuentes aceptadas; reemplazo
por commit normal sobre db88286, sin reescritura/push. Antecedente SDD intacto salvo
su estado/propuesta ya marcados sustituidos. Referencias: UM10360 rev.2 §21 y errata
NXP 10.6 PCLKSELx.1 consultadas 2026-11-10. PCLK real/placa/revisión físicos pendientes.

Adaptación de verificación: B00 S7 contaba 18 objetos. Tiempo añade el número 19;
exigir conjunto exacto independiente de nombres y cantidad, sin quitar controles.
PCONP requiere gestion de alimentacion serializada con otros propietarios; se
conservan bits ajenos mediante read-modify-write bajo esa precondicion.

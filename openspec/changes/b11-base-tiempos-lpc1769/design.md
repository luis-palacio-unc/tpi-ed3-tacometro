# Diseño B11 — Base de tiempos

## Decisiones

| Opción | Decisión y motivo |
|---|---|
| SysTick / timer de propósito general | SysTick exclusivo, sin pines; deja timers libres para captura. No planificador. |
| Frecuencia asumida / explícita | Parámetro Hz verificado por integrador, estable. 2000..120000000, múltiplo de 1000; LOAD=Hz/1000-1 (1..119999). LOAD cero no genera ticks periódicos. |
| Instancias / único contexto | Un periférico único; contador y estado privados static volatile, sin heap. No globals mutables en servicios. |
| Lectura libre / sección crítica | PRIMASK preservado, instantánea de estado/contador protegida; APIs sólo desde lazo privilegiado serializado, nunca ISR/NMI/RTOS concurrente. |
| Puerto completo simulado / acceso mínimo | Tres funciones privadas: EntrarCritica, SalirCritica y Escribir(registro,valor). Secuencia real compartida; modelo host observa orden. Adaptador ARM mapea cinco operaciones a CMSIS existente. |

## API y estados

Base_Tiempos_Inicializar(uint32_t frecuencia_cpu_hz), Leer(Base_Tiempos_Lectura*) y
Detener(void) retornan Base_Tiempos_Estado. Lectura contiene milisegundos uint32_t
y estado. SIN_DATOS=0, ACTIVA, DETENIDA, ERROR_ARGUMENTO, ERROR_FRECUENCIA.
ERROR_ARGUMENTO sólo es retorno de Leer(NULL), sin efectos ni sección crítica.
Sólo ACTIVA habilita uso como tiempo actual; DETENIDA entrega último contador congelado.
SIN_DATOS/ERROR_FRECUENCIA entregan cero. Estado privado nunca ERROR_ARGUMENTO.
SysTick_Handler(void) incrementa sólo ACTIVA; no lee CTRL/COUNTFLAG ni escribe registros.

Inicializar siempre descarta época anterior; configuración inválida deshabilita y
limpia a cero/ERROR_FRECUENCIA. Detener cambia sólo ACTIVA→DETENIDA; conserva otros
diagnósticos y contador. Reinicio válido devuelve cero/ACTIVA. Consumidores deben
reiniciarse al cambiar época; el tiempo no equivale a órdenes B01 uint64_t.

## Secuencia y concurrencia

Entrar: guardar PRIMASK, CPSID i, barrera de compilador y DSB/ISB. El CMSIS antiguo
carece de clobber memory en intrínsecos; añadir barreras de compilador en el puerto,
sin modificar Drivers. Salir: barrera compilador/DSB, restaurar PRIMASK, ISB/barrera.

Inicializar, bajo máscara: CTRL=0 → VAL=0 → ICSR=PENDSTCLR; cero/ERROR_FRECUENCIA.
Si frecuencia válida: LOAD=Hz/1000-1 → prioridad SysTick 31 (mínima, cinco bits)
→ estado ACTIVA → CTRL=7 (CPU/TICKINT/ENABLE); restaurar máscara. ICSR se escribe
directamente, nunca read-modify-write. No modificar prioridades ajenas/AIRCR.
Detener hace misma secuencia de apagado; conserva tiempo. Leer copia estado/tiempo
a locales bajo máscara y escribe destino después de restaurar: un tick pendiente
puede ejecutarse después del snapshot, que permanece coherente. ISR ve datos
completos; interrupciones pendientes anteriores se cancelan al apagar.

Ticks no atendidos pueden colapsar. Integrador debe garantizar latencia/cadencia,
CPU estable y ausencia de suspensión/debug durante intervalos que desea medir.
Sin recuperación/detección de múltiples ticks perdidos ni precisión física acreditada.

## Archivos y documentación

- Dispositivos/soporte/Base_Tiempos.h/.c: API/estado/ISR/helper de apagado.
- Base_Tiempos_Puerto.h y Base_Tiempos_LPC1769.c: contrato privado y backend ARM.
- CMake: backend sólo ARM; host OBJECT admite referencias externas; tests proveen puerto.
- tests/host/: modelo de registros, prueba principal, integración B10 y prueba del
  backend real contra encabezado CMSIS mínimo de test. Backend también compila con CMSIS real en ARM.
- cmake/verificar_enlace_b11.cmake y scripts/verificar_vector_b11.py: fixture vivo
  y comprobación del vector 15 apuntando al handler fuerte, sin ejecutar MCU.
- Doxyfile/README y seguimiento SDD. ESW.2.1.17 es extensión local de soporte,
  no módulo impuesto por anexo B (termina en .16). Ocho USW .17.1–.8.

## Verificación y límites

Modelo host: registros/pending/máscara, entrega IRQ al restaurar, traza de accesos;
se prueban fronteras, parada/reinicio y tick entre snapshot/salida. Rollover se
siembra sólo en test incluyendo el .c; sin API/condicional de prueba en producción.
Backend host: intrínsecos/registros dobles, verificar mapa y conservación de campos.
Prueba B10 consume sólo ACTIVA, frontera y reinicio de consumidor entre épocas.
Regresiones/san, ARM Debug/Release, Doxygen estricto, tamaño/pila/desensamblado y
negativos por omitir limpieza pendiente o usar recarga incorrecta.

Fuentes consultadas 2026-11-09: UM10360 rev.2 §23/34.4.3.4/34.4.4, CMSIS existente,
erratas NXP 10.6 (sin entrada específica SysTick encontrada). Frecuencia física,
precisión y aceptación en placa pendientes. No migración; main/startup/linker no
cambian, ninguna llamada real inicializa el periférico todavía.

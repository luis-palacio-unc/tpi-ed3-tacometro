# Diseño B02

## Decisiones

| Elección | Alternativa | Motivo |
|---|---|---|
| Buffer uint8_t[8], longitud explícita | C strings/atoi | No depender de terminador NUL/libc ni convertir más de tres dígitos |
| Cola circular de 4 eventos, descartar nuevo si llena | Sobrescribir viejo | Conservar FIFO y hacer observable la pérdida |
| Errores enum por llamada | Silenciar/contadores globales | El integrador recibe causa sin añadir estado global |
| Marca del byte LF, nunca al extraer | Fecha de parseo/consumo | Mantener antigüedad que exige B01 |
| Configuracion/Limites.h con TPI_RPM_MAX=185U | Servicio incluye Procesamiento.h | Una fuente de rango sin invertir capas; alias B01 se conserva |
| Contexto explícito sin heap | Singleton/buffer dinámico | Instancias aisladas y memoria acotada |

## API y flujo

Comunicacion_Inicializar(ctx, orden_inicial) devuelve bool y purga cola/trama.
Comunicacion_RecibirByte(ctx, byte, orden) devuelve enum: OK para byte parcial o
cola de descarte, ENCOLADA, o ERROR_ARGUMENTO/ORDEN/LONGITUD/FORMATO/RANGO/COLA_LLENA.
Comunicacion_ExtraerConsigna(ctx, evento) devuelve EXTRAIDA/VACIA/ERROR_ARGUMENTO.
Salida escribible se limpia ante vacío/error; un evento NULL no consume la cola.

Eventos del servicio: {uint32_t rpm; uint64_t orden_recepcion}. No incluye tipos de
aplicación. El adaptador vive en aplicación/test y fija valida=true sólo para eventos
extraídos. La API no integra todavía dispositivos ni el main real.

Contexto: cuatro eventos, último orden, buffer de ocho bytes, longitud/cabeza/cantidad
uint8_t y bandera de descarte. Sólo leer/escribir cola dentro de cantidad válida.
Inicializar metadatos; no hace falta borrar bytes fuera de longitud/entradas no ocupadas.
Parámetros deben ser memoria válida sin solapamiento; contexto siempre inicializado.

Cada byte recibido lleva orden global único > último, compartido con eventos de
modo al integrar el sistema. No es tiempo ni un número recibido desde PC. Las llamadas
y captura se serializan externamente; B02 no implementa atomicidad/ISR de 64 bits.
No incrementar internamente ni permitir rollover. Reiniciar implica nueva sesión y
reinicialización coordinada de B01/colas. Rechazo por orden conserva marca confiable,
invalida trama parcial y descarta hasta LF (incluye el byte rechazado); FIFO previa
permanece. Tras LF basta volver a un orden mayor al último confiable.

Para byte válido: actualizar marca; si se descarta, esperar LF; si LF, decodificar y
reiniciar trama, luego encolar si válida y hay lugar. Otro byte: almacenar sólo si
longitud<8; si no, error LONGITUD una vez y descartar hasta LF. No reinterpretar sufijos.
Decodificar: CR sólo al final, longitud efectiva 5..7, prefijo exacto, 1..3 dígitos,
acumulación uint32_t acotada a 999 y comparación con máximo común. Sin isdigit/locale.

## USW y archivos

ESW.2.1.7: .1 Inicializar, .2 RecibirByte, .3 ExtraerConsigna, .4 ReiniciarTrama,
.5 Decodificar y .6 Encolar. Contratos/IDs Doxygen en header y helpers en .c.
Configuracion/Limites.h: dato común, no nueva capa o módulo de comportamiento.
Procesamiento.h: alias público de máximo y include común, sin cambio de lógica B01.
CMake y fixture B01: agregar include de Configuracion para mantener API compilable.
Tests nuevos en tests/host/test_comunicacion.c; fixture ARM B02 fuerza enlace del
servicio con B01 y arranque mínimo, sin libc. Doxyfile amplía INPUT/INCLUDE_PATH.

## Verificación

Pruebas de S1–S8 por retornos y eventos (incluye todos los bytes posibles en campo
numérico, longitudes, cola circular, marcas, NULL y UINT64_MAX). Integración con B01
para datos antiguos/nuevos. Host normal/sanitizadores; builds ARM Debug/Release y
fixture de enlace (-O0 Debug, -O3 Release); B00/B01 sin cambios de comportamiento. Doxygen estricto HTML/XML
con seis USW nuevas. Revisar memoria/stack/diff y preservar archivos archivados.
Sin hardware ni tiempos reales. No hay decisiones bloqueantes dentro de este alcance.

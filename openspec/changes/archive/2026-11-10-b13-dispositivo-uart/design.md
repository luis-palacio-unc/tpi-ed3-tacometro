# Diseño candidato B13 — UART

## Base y decisiones

UART existente es sólo esqueleto, ESW.2.1.16 según anexoB. Comunicacion ya recibe
bytes/codifica tramas; no se cambia. Inicialización de periféricos e integración
se difieren por pedido vigente del usuario. No confundir el comentario inicial
de UART.h sobre buffers/interrupciones con una obligación de este bloque.

| Decisión | Razón |
|---|---|
| Dos funciones UART_Read/UART_Write con uint32_t de progreso | Reflejan conteo real de driver y permiten binario/envío parcial sin estados ficticios. |
| UART0 acordado, macro del driver privada en .c | Recurso concreto en Dispositivos; evita otra capa genérica/proveedor/contexto. |
| Una llamada a Receive/Send con NONE_BLOCKING | La biblioteca ya resuelve FIFO, comprobaciones de disponibilidad y bucles. |
| Guardar NULL/tamaño0 antes de delegar | Evita acceso inválido sin crear diagnósticos que hardware no proporciona. |
| Sin colas/IRQ/init ahora | Mantiene el bloque verificable y respeta integración/configuración finales. |

## Implementación prevista

UART.h incluye tipos estándar; no nuevas estructuras, callbacks ni inicializadores.
UART_Read(destino,capacidad): si NULL/cero,0; caso válido UART_Receive(UART0,
destino,capacidad,NONE_BLOCKING), devolver conteo sin conversión.
UART_Write(datos,longitud): misma validación y UART_Send(UART0,datos,longitud,
NONE_BLOCKING). Sin comprobar busy, leer registros, duplicar FIFO o reenviar resto.
UART_Write es const en origen, como el driver original. No strings/terminadores.

Entrada: buffer+tamaño; salida: bytes leídos/progreso TX. El consumidor mantiene
vida/capacidad reales, serialización y el resto de una transferencia parcial. Las
funciones no retienen buffers ni configuración; su duración depende del tamaño,
no hay garantía de tiempo constante. Cero también significa ausencia de progreso;
no sirve para diferenciar error de línea ni confirma recepción por la PC.

Precondiciones físicas: UART0 configurada, reloj correcto, pines habilitados,
DLAB=0, RX/TX/FIFO activos, sin otro escritor/lector ISR/DMA. TX del driver escribe
ráfagas hasta16 bytes al ver THRE; configurar FIFO antes de usar. Baudios/formato,
cableado y conversor pendientes. TX P0.2/RX P0.3 asignados, función01 según
manual tabla80/p118 revisada en imagen; el driver ofrece UART_PinConfig para
configurarlos al integrar. No configurar ahora ni duplicar PINSEL.

Aclaración autorizada: UART_BAUDRATE en UART.h, default115200 con #ifndef para
override de compilación. Validar >0 y <=UINT32_MAX con #if/#error (sin _Static_assert
en header). Es un parámetro preparado para el UART_CFG_T.baudRate futuro;
no es baudrate aplicado ni acredita divisores/error con reloj real. No añadir
init/getter/setter, más archivos de configuración o macros de pines duplicadas.

## Archivos al implementar

- Dispositivos/UART.h/.c: dos USW.2.1.16.1/.2 con Doxygen, sin comentarios internos.
- tests/host/test_uart.c: doble sólo de UART_Send/UART_Receive con headers originales;
  colas/capacidad en el modelo host, no implementarlas como firmware.
- CMakeLists.txt: target/test propio con avisos estrictos y headers de biblioteca;
  el módulo ya integra lista de fuentes, conservar su cantidad.
- docs/doxygen/Doxyfile y README: ampliar INPUT, evidencia real; docs/agente y cambio SDD.

## Verificación prevista

S1: NULL/tamaño0/canarios/recuperación sin llamar driver. S2/S3: RX vacío, parcial,
completo, cero embebido/0xFF y resto de destino sin modificar. S4/S5: TX0/parcial/
completo y continuación a partir de offset por el test, sin duplicados/cambios en origen.
El doble comprueba UART0 y NONE_BLOCKING y simula sólo API de transferencia.
No imitar registros ni alterar compilación/código de biblioteca para ejecutar host.

Host/sanitizadores, regresiones y objetos ARM con headers originales; aislar header,
revisar recursos propios/símbolos y Doxygen estricto. No enlazar conjunto de biblioteca
o main ahora. No atribuir al test de doble el comportamiento eléctrico del driver.

Alcance/UART0 acordados2026-11-10. Verificar default/overrides de UART_BAUDRATE
y rechazar0/negativo/exceso. Aplicación de baudios/pinmux al hardware sigue pendiente.
Hardware, errores de línea, cadencia/colas y aplicación se resuelven al integrar.

Implementado/verificado según este diseño; resultado pendiente de aceptación.
Pruebas realmente ejecutadas y límites en verify-report.md. No afirmar configuración
de pines/baudios aplicada por haber asignado el recurso o compilado el parámetro.

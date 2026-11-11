# B13 — Lectura y escritura UART mediante el driver existente

Estado: alcance/UART0 acordados2026-11-10, implementación autorizada; resultado pendiente de aceptación.

## Objetivo y origen

R09/R10/R11/R15 del anteproyecto y pedido de completar Dispositivos antes de integrar.
Desarrollar UART (ESW.2.1.16): mover bytes entre buffers del llamador y UART0,
reutilizando UART_Receive/UART_Send con NONE_BLOCKING, sin protocolo en el dispositivo.

## Alcance

- UART_Read(uint8_t *destino,uint32_t capacidad): devuelve cantidad recibida.
- UART_Write(const uint8_t *datos,uint32_t longitud): devuelve cantidad entregada
  al transmisor/FIFO, sin confirmar recepción por PC ni fin físico del envío.
- Ambas retornan uint32_t:0..longitud solicitada. Admitir progreso parcial/cero;
  NULL o longitud cero devuelve0 sin acceder al driver/buffer. Sin contexto.
- Una delegación al driver por llamada válida, sin reintentos/esperas añadidos,
  buffers propios, heap, estado global, ISR o lecturas/escrituras de registros propias.
- Doxygen USW.2.1.16.1/.2, pruebas host, compilación ARM y regresiones.

Archivos: Dispositivos/UART.h/.c, tests/host/test_uart.c, CMakeLists.txt,
docs/doxygen/Doxyfile, README y seguimiento SDD. Drivers/servicios/otros dispositivos intactos.

Excluye inicialización UART/PINSEL/reloj/FIFO y aplicar baudios/formato físico, buffers/IRQ/DMA,
timeouts/diagnóstico de línea, conexión a Comunicacion/main y ensayos físicos.
Aclaración del usuario: asignar P0.2/TXD0 y P0.3/RXD0 confirmados en manual;
UART_BAUDRATE configurable en UART.h (inicial115200), aplicado al inicializar al final.
El comentario del esqueleto sobre buffers/interrupciones es una intención previa,
no requisito implementado; se resolverá al integrar si es necesario.

## Dependencias, pendientes y aceptación

UART0 y pines P0.2/TXD0/P0.3/RXD0 quedan asignados; PINSEL0 función01
según manual tabla80/p118, verificado en imagen. No se configura hardware ahora. Uso físico requiere reloj,
pines, UART/FIFO habilitados y DLAB=0; sin otro consumidor ISR/DMA concurrente.
El parámetro de baudios se deja preparado; trama, cableado/conversor y cadencia
se acordarán en integración final, junto con aplicar su configuración física.

1. Lecturas/envíos completos, parciales y cero conservan orden/bytes binarios y
   devuelven el progreso real; RX preserva canarios/cola, TX no modifica origen.
2. NULL/tamaño cero no accede a hardware ni memoria; una llamada válida posterior funciona.
3. Se usa driver original NONE_BLOCKING, sin lógica duplicada ni cambiar servicios.
4. Host/sanitizadores, compilación ARM, regresiones/Doxygen pasan; biblioteca intacta.

Riesgo: confundir progreso parcial con mensaje completo. Documentar que el consumidor
conservará el resto/offset y decidirá nueva llamada al integrar, sin agregarlo ahora.
Reversión: commit inverso sólo B13. Commit al terminar alcance acordado, sin push.

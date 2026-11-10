# Verificación prevista B13 UART

Propuesta sin acuerdo ni implementación. Ninguna prueba B13 ejecutada.
Exploración de archivos/manual confirma firmas del driver y soporte no bloqueante;
no demuestra UART funcionando ni los wrappers propuestos.

| Escenario | Evidencia esperada al implementar |
|---|---|
| S1 |NULL/cero sin invocar driver ni tocar memoria; siguiente petición válida funciona |
| S2/S3 |RX completo/parcial/vacío/binario; orden/canarios/cola intactos |
| S4/S5 |TX completo/parcial/cero; origen inmutable, continuación externa sin duplicar |
| S6 |Host/san, objeto ARM original, Doxygen/recursos/diff/hashes; sin ensayo eléctrico |

Comandos previstos: cmake -S "$R" -B "$O/host", build y ctest; build sanitizado
con -DTPI_HOST_SANITIZERS=ON y detección de fugas; ARM con -DTPI_BUILD_ARM=ON y
toolchain cmake/arm-none-eabi.cmake; Doxygen con OUTPUT_DIRECTORY externo.
Registrar comandos/exits/logs reales fuera del árbol al ejecutar.

No aceptar funcionamiento físico por un doble host. Configuración/cableado/baudios,
enlace global y aplicación/Comunicacion permanecen pendientes de integración final.

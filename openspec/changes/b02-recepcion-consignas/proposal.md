# B02 — Recepción y validación de consignas

Estado: propuesto, sin autorización de implementación.

## Objetivo y origen

Implementar la recepción lógica de consignas en Comunicacion (ESW.2.1.7), conservando
la marca de recepción que necesita B01. Origen: R09 (consignas desde PC), R14
(0..185 RPM) y soporte de R06 (descartar consignas anteriores al ingreso automático).

## Alcance propuesto

- Protocolo ASCII inicial: `RPM <decimal>\n`, también terminación CRLF; 1..3 dígitos
  sin signo ni caracteres extra. Ejemplos: `RPM 0\n`, `RPM 185\n`.
- Procesamiento incremental por bytes: tramas fragmentadas o consecutivas.
- Buffers/cola acotados, sin heap; errores explícitos y recuperación tras desborde.
- Evento propio del servicio con RPM y orden global al completar la trama recibida,
  preservado al encolarlo/extraerlo. La aplicación adaptará el evento a B01.
- Documentación Doxygen de cada USW y pruebas C host con integración lógica B01.

Excluye UART/GPIO reales, configuración de ISR, telemetría, respuestas por UART,
GUI de PC, PWM/controlador, temporización física e integración del main de producción.

## Dependencias y decisiones

B00/B01 aceptados. Antes de implementar: fijar capacidades de buffers/cola, API de
notificación de errores y acceso serializado. Compartir/configurar el máximo 185 sin
crear dependencia Servicios→Aplicacion. No se necesitan pines para estas pruebas.
La marca se asigna al completar recepción, nunca al extraer de cola ni desde datos PC.

## Archivos previstos

Servicios/Comunicacion.c/.h, tests/host/, CMakeLists.txt y Doxyfile; configuración de
límites sólo si el diseño la necesita. Sin cambios en drivers UART o archivo B01.

## Escenarios y aceptación

1. 0 y 185 producen exactamente un evento válido por trama, incluso byte a byte o
   con varias tramas consecutivas; se conserva el orden de recepción al consumir.
2. Negativos, 186, texto no numérico, exceso de dígitos, tramas incompletas o mal
   formadas no generan consignas aplicables ni desbordes/conversiones indefinidas.
3. Exceso de longitud/cola llena: error observable, sin sobrescribir memoria ni
   reinterpretar restos; la siguiente trama válida tras resincronizar se procesa.
4. Prueba integrada con B01: una trama recibida antes de entrar en automático y
   consumida después no lo habilita; una nueva trama posterior sí puede hacerlo.
5. C host/sanitizadores pasan; ARM compila/enlaza, B00/B01 sin regresiones y Doxygen
   estricto correcto. Evidencia por escenario, sin afirmar UART físico funcional.

## Riesgo y reversión

Riesgos: perder marcas o depender de temporización/hardware en el parser.
Revertir sólo cambios propios B02, preservando contratos y archivos aceptados.

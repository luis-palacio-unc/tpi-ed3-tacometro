# B09 — Codificación portable de telemetría

Estado: resultado aceptado el 2026-11-09. Sincronización, archivo y publicación
autorizados; propuesta B10 solicitada sin implementación.

## Objetivo y origen

R09 requiere transmitir RPM, temperatura y estado a PC. B02 implementa recepción;
B03/B05 entregan magnitudes/validez y B01, estado de operación. Preparar la trama
sin UART concreta permite verificar esta parte bajo R10/R15.

## Alcance y comportamiento propuesto

- Extender Comunicacion con codificación ASCII hacia un buffer del llamador,
  capacidad explícita, longitud producida y resultado. Sin heap, flotantes ni esperas.
- Formato propuesto: `RPM=123.456;TEMP=25.34;ESTADO=MANUAL\n`.
  RPM conserva tres decimales; temperatura, dos y signo. ESTADO admite MANUAL,
  AUTO_ESPERA o AUTO_ACTIVO. LF termina la trama; NUL adicional no integra la longitud.
- Lecturas ausentes/erróneas se representan ND/ERR, diferenciadas de cero válido.
  Recibir magnitudes/estados lógicos propios del servicio; adaptar B01/B03/B05 en
  pruebas, sin dependencia Servicios→Aplicacion ni modelo de sensor/placa.
- Validar argumentos/estados y capacidad; error explícito y longitud cero, sin trama
  parcial utilizable. Si el buffer permite escribir, dejar cadena vacía ante error.
  Una solicitud válida posterior recupera; entradas/configuración no se modifican.
- Conservar recepción/cola B02. Su exclusión de telemetría B02-R5 corresponde al
  alcance original: B09 propone ampliarla al aceptar su resultado, manteniendo el resto.

Excluye UART/ISR/DMA reales, envío/colas TX, periodicidad, aplicación PC, aviso térmico
opcional, adquisición, regulación, main y nuevas políticas de fallo. Codificar no acredita envío.

## Dependencias, archivos y decisiones

B00–B08/B03-R1 aceptados. Servicios/Comunicacion.h/.c, pruebas host nuevas y adaptación
B01/B03/B05, fixture ARM, CMake, Doxygen ESW.2.1.7/USW y seguimiento SDD.
Sin dependencias nuevas. Formato anterior es decisión propuesta, no consigna original;
API exacta y capacidad máxima se derivarán en especificación/diseño tras acuerdo.

## Criterio de aceptación

1. Tramas exactas para estados de operación, cero, decimales, temperatura negativa
   y ND/ERR; conservar medidas mayores que 185 RPM.
2. Probar UINT32_MAX de RPM e INT32_MIN/MAX de temperatura sin desbordes;
   NULL/enums inválidos, buffer vacío/corto/exacto, canarios, recuperación e instancias.
3. Demostrar adaptación de contratos genéricos y preservar todas las pruebas B02.
4. Pasar host/sanitizadores, regresiones, enlace ARM y Doxygen estricto; distinguir
   codificación lógica de transmisión y pruebas físicas pendientes.

## Riesgos y reversión

Confundir ausencia con cero o truncar una trama: estados explícitos y pruebas de
fronteras. Reversión futura mediante commit inverso del bloque, preservando historial/trabajo ajeno.

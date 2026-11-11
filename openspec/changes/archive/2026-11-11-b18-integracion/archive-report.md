# Cierre B18 — 2026-11-11

El usuario ordena cerrar la integración, sincronizar, archivar y publicar `main`,
sobre e3d3bce (I1 ff07e3e, I2 e3a279c, I3 a5a888e, I5 e3d3bce). Implementación de
Claude por pedido explícito; I1–I3 con implementador y revisor separados, I5
implementado y verificado por el mismo agente.

## Qué se acepta y qué no

Aceptado: firmware integrado verificado en host y por compilación y enlace ARM
(CMake y MCUXpresso). **No aceptado físicamente:** nada se ejecutó en la placa. La
puesta en marcha (I4, tarea 3.9) queda pendiente a cargo del usuario; los ajustes
que surjan irán en un cambio nuevo. Pendientes conocidos: sintonía de las ganancias
del PI, cantidad de ranuras del disco (`TPI_MEDICION_PULSOS_POR_VUELTA`, hoy 60),
medición de VREFP y telemetría parcial si el buffer de transmisión se llena.

## Sincronización y archivo

17 deltas aplicados a `openspec/specs/`:

- Reemplazo completo: alarma-temperatura (AT5), comunicacion (CO1–CO3), mando-motor
  (MM5), mandos (MA5–MA6), medicion-velocidad (MV1–MV3), modos (MO1–MO2),
  regulacion-velocidad (RG1–RG5) y salidas-alarma (SA5).
- Modificados y ampliados: build (B18-R5 a R7), dispositivos-gpio (DG3, DG4),
  motor-dispositivo (MD3, MD5), potenciometro-dma (PD2, PD4, PD5), sensor-optico
  (SO2, SO3, SO5), temperatura (TE1, sin TE2, TE5), timer (WT1, WT2, WT4) y
  uart-dispositivo (UD1–UD4, UD5, UD6).
- Nuevo: sistema (SI1–SI4).

El cuerpo de cada requisito es el del delta, sin cambios; sólo se agrega la nota de
estado en la cabecera. Los contratos reemplazados quedan en los cambios archivados
anteriores, que no se editan.

## Evidencia

[verify-report](verify-report.md), secciones I1, I2, I3 e I5. Última ejecución sobre
e3d3bce: host con sanitizadores 29/29 y sin ellos 29/29; ARM Debug 9/9 y Release
9/9; Doxygen 58 funciones sin avisos; MCUXpresso Debug y Release con 0 errores y 0
avisos; `Drivers/` sin cambios. El cierre sólo cambia documentación y specs; se
repiten host y ARM después de sincronizar.

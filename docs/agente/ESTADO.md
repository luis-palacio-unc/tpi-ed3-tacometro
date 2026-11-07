# Estado — 2026-11-06

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama main; base B04 db0c305b8f2533884c00e416f52c3dd75acf652b, publicada en origin/main.
  Revisión al reanudar: `git rev-parse HEAD`; sincronización: `git status -sb`.
- B00–B03 aceptados/archivados; contratos vigentes preservados, no editar historial.
- B04 acordado, implementado/verificado; resultado pendiente de aceptación.
  Cambio activo: openspec/changes/b04-normalizacion-mando-manual/.
- Cierre local con asunto `feat: implementar B04 de normalizacion del mando manual`;
  consultar `git log -1` para hash. Commit autorizado; push explícitamente no autorizado.
- Propuesta/contexto locales previos incorporados al bloque. Specs B04 candidatas;
  sin promoción ni archivo hasta aceptación. Hooks/firma conservados.
- main sigue mínimo; sin servicios integrados al hardware ni aceptación física.

## Evidencia ejecutada B04

| Verificación | Resultado |
|---|---|
| CTest host | 4/4; B01 17 + B02 17 + B03 15 + B04 15 = 64 casos |
| ASan/LeakSanitizer/UBSan | 64 casos sin hallazgos, fuera del sandbox/ptrace |
| Normalización | 100 fronteras, 10 000 pares y 71 680 puntos de recorrido; redondeo, límites, monotonía y recuperación |
| ARM Debug / Release | CTest Debug 6/6 (B00 14 casos); fixture B04+B01 Release correcto |
| Doxygen HTML/XML estricto | Sin avisos; 2 USW nuevas Mandos, 17 funciones totales y límite/aliases documentados |
| Recursos B04 -O0 | Salida 4 bytes, código 292 bytes sin data/bss; stack interno hasta 136 bytes con libgcc |
| ELF fixture | Sin indefinidos; API y auxiliar retenidos; GNU_STACK RW |
| Revisión | Archivos aceptados, main, lógica B01–B03 y pruebas previas intactos; sólo alias B01 al mismo 1000 |

Matriz S1–S7, comandos y limitaciones: verify-report.md del cambio activo.
Logs extensos: /tmp/ed3-b04-20261106-01ljgnod (temporales, fuera de Git).
Sin fallos observados. Sin ejecución ARM, simulador ni placa; no se flasheó,
instaló ni configuró hardware. Memoria/pila son inspección estática.

## Decisiones y limitaciones

- Mandos recibe lectura/escala uint32_t; salida uint16_t 0..1000 y estado explícito.
  Escala configurable por llamada, cercano/empate arriba, sin bits/VREF/canal fijos.
- Prioridad errores: salida NULL, escala cero, lectura excesiva. Error escribible
  borra valor viejo; sólo VALIDO permite consumo. Sin timeout de frescura.
- Límite manual común TPI_MANDO_MANUAL_MAX en Configuracion/Limites.h; B01 conserva
  alias público y comportamiento. No hay dependencia Servicios→Aplicacion.
- R15: aplicación portable, datos lógicos y adaptación física en capas inferiores.
- Consignas siguen 0..185; medición B03 puede superarlas. Ventana física no fijada.
- Faltan ADC/DMA, selector/antirrebote, captura, UART real, telemetría, PWM/control,
  alarmas y main. Pines, revisión PCB/silicio, VREF y reloj pendientes; CMSIS-DAP integrado.
- Sin CI ni documentación final completa; Doxyfile cubre módulos desarrollados.
- AGENTS/.atl locales por ignore global; procedimiento compartido en docs/agente/.

## Próxima acción

Revisar aceptación del resultado B04 con el usuario. Sólo tras ella y orden de cierre,
sincronizar specs y archivar. No iniciar B05 ni hacer push sin autorización.

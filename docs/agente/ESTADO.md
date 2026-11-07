# Estado — 2026-11-07

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama main; base B05 8d5bc7b040fc205af48a00db35de5f7622c82379, publicada en origin/main.
  Revisión al reanudar: `git rev-parse HEAD`; sincronización: `git status -sb`.
- B00–B04 aceptados/archivados; contratos e historial preservados, no editar archivos aceptados.
- B05 acordado, implementado/verificado; resultado pendiente de aceptación.
  Cambio activo: openspec/changes/b05-conversion-temperatura/.
- Cierre local con asunto `feat: implementar B05 de conversion de temperatura`;
  consultar `git log -1` para hash. Commit autorizado; push explícitamente no autorizado.
- Propuesta/contexto locales previos incorporados; specs B05 candidatas, sin promoción
  ni archivo hasta aceptación. Hooks/firma conservados. main mínimo, sin integración física.

## Evidencia ejecutada B05

| Verificación | Resultado |
|---|---|
| CTest host | 5/5; 17 B01 + 17 B02 + 15 B03 + 15 B04 + 17 B05 = 81 casos |
| ASan/LeakSanitizer/UBSan | 81 casos sin hallazgos, fuera del sandbox/ptrace |
| Aritmética B05 | 1000 fronteras, 10000 ternas y 70678 puntos de recorrido; equivalencia, límites y redondeo |
| Controles negativos temporales | Suma con overflow y doble redondeo rechazados por S6/S3; fallos esperados |
| ARM Debug / Release | CTest Debug 7/7 (B00 14 casos); fixture B05 Release correcto |
| Doxygen HTML/XML estricto | Sin avisos; 2 USW Temperatura nuevas y 19 funciones totales |
| Recursos -O0 | Lectura 8 bytes, código 390 bytes sin data/bss; stack interno hasta 176 bytes con libgcc |
| ELF fixture | API/auxiliar retenidos, sin indefinidos; GNU_STACK RW |
| Revisión | main, B01–B04, Configuracion, drivers/dispositivos, pruebas y archivos aceptados intactos |

Matriz S1–S8, comandos y límites: verify-report.md del cambio activo.
Logs extensos: /tmp/ed3-b05-20261107-e27t9yaq (temporales, fuera de Git).
Sin fallos pendientes. No se ejecutó ARM en MCU/simulador ni prueba física, no se
flasheó, instaló o configuró hardware. Pila/recursos son inspección estática.

## Decisiones y limitaciones

- Temperatura recibe muestra/escala/referencia µV uint32_t; salida uint32_t centésimas
  Celsius y estado. Escala son cuentas por referencia; muestra<escala. Ej.: 4096/0..4095.
- B04 conserva fondo de escala máximo inclusivo para mando; no cambiar ni confundir
  los dos contratos. Aplicación mantiene magnitudes lógicas (R15).
- Modelo ideal LM35 sin offset, redondeo cercano/empate arriba por cociente/resto;
  sin desborde al redondear ni cuantización intermedia. No se impone rango físico no confirmado.
- Prioridad: NULL, escala cero, referencia cero, muestra excesiva. Error escribible
  borra dato viejo; sólo VALIDA permite consumo numérico. Sin timeout de frescura.
- B03 puede medir sobre 185 RPM; consignas B01/B02 siguen 0..185. Ventana física no fijada.
- Pendientes ADC/DMA, selector, captura, UART/telemetría, PWM/control, alarmas y main;
  pines, revisión PCB/silicio, VREF, variante/montaje LM35 y reloj aún sin confirmar.
- CMSIS-DAP integrado confirmado. Sin CI/aceptación física; documentación final pendiente.
  AGENTS/.atl locales por ignore global; procedimiento compartido en docs/agente/.

## Próxima acción

Revisar aceptación del resultado B05. Sólo tras ella y orden de cierre, sincronizar
specs y archivar. No iniciar B06 ni hacer push sin autorización.

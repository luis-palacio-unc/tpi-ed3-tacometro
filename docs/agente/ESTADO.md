# Estado — 2026-11-06

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama main; implementación B04: 915b614b77d6883b165e279e960f081859cb3472.
  Revisión al reanudar: `git rev-parse HEAD`; sincronización: `git status -sb`.
- B00–B04 aceptados/archivados; contratos build, modos, comunicación, medición y mandos vigentes.
- Archivo B04: openspec/changes/archive/2026-11-06-b04-normalizacion-mando-manual/;
  no modificar historial. Cuerpo normativo sincronizado sin cambios.
- Usuario aceptó B04 y autorizó publicar implementación y cierre. Commit de cierre:
  el que contiene este ESTADO; consultar Git. Verificar resultado del push al reanudar.
- B05 pendiente de propuesta escrita, sin acuerdo ni implementación: conversión lógica
  de temperatura para futura alarma/telemetría. Prepararla tras publicar B04.
- main sigue mínimo; servicios sin integración física. Hooks/firma conservados.

## Evidencia aceptada B04

| Verificación | Resultado |
|---|---|
| Host / sanitizadores | 64 casos: B01 17 + B02 17 + B03 15 + B04 15; sin hallazgos ASan/LeakSanitizer/UBSan |
| Normalización | 100 fronteras, 10 000 pares y 71 680 puntos de recorrido: límites, redondeo, monotonía y recuperación |
| ARM Debug / Release | CTest Debug 6/6 (B00 14 casos); fixture Mandos/B01 Release correcto |
| Doxygen estricto | Sin avisos; 17 funciones/USW, límite común y aliases documentados |
| Recursos estáticos B04 | Salida 4 bytes; código -O0 292; stack interno hasta 136 bytes con libgcc |
| ELF fixture | Sin indefinidos; API/auxiliar retenidos y GNU_STACK RW |
| Cierre documental | Sin cambios de firmware; requisitos, hashes, YAML y vínculos comprobados |

Matriz S1–S7 y comandos en verify-report.md archivado; aceptación/manifest en
archive-report.md. Logs implementación: /tmp/ed3-b04-20261106-01ljgnod.
Logs cierre: /tmp/ed3-cierre-b04-20261106-gt6twg61 (temporales, fuera de Git).
No se repiten pruebas de firmware en cierre documental. Sin ejecución ARM,
simulación ni placa; no se flasheó, instaló ni configuró hardware.

## Decisiones y limitaciones

- Mandos: lectura/escala uint32_t, mando uint16_t 0..1000; cercano/empate arriba y
  validez explícita. Límite común y alias B01 conservan comportamiento; sin dependencia hacia aplicación.
- R15: aplicación portable entre configuraciones físicas; ver CONTEXTO.
- B03 mide sin recorte a 185 RPM; consignas B01/B02 siguen 0..185. Ventana física no fijada.
- Faltan ADC/DMA, selector/antirrebote, temperatura, captura, UART real, telemetría,
  PWM/control, alarmas y main. Pines, revisión PCB/silicio, VREF y reloj pendientes.
- CMSIS-DAP integrado confirmado; sin CI ni aceptación física. Doxyfile parcial,
  documentación final pendiente. AGENTS/.atl locales por ignore global.

## Próxima acción

Publicar commits B04 autorizados y preparar propuesta B05 con alcance y aceptación.
No implementar B05 ni extender autorización de push a bloques futuros sin acuerdo.

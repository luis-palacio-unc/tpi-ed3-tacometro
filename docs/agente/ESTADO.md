# Estado — 2026-11-06

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama main; implementación B03: e7ba0183c290f9e5384f609d820a06752587966a.
  Revisión al reanudar: `git rev-parse HEAD`; sincronización: `git status -sb`.
- B00–B03 aceptados/archivados. Contratos build, modos, comunicación y medición vigentes.
- Archivo B03: openspec/changes/archive/2026-11-06-b03-medicion-velocidad/;
  no modificar historial archivado. Cuerpo normativo sincronizado sin cambios.
- Usuario aceptó B03 y autorizó publicar implementación y cierre documental.
  Commit de cierre: el que contiene este ESTADO; consultar `git log -1`.
  Verificar resultado real del push al reanudar; no inferir permisos futuros.
- B04 pendiente de propuesta escrita, sin acuerdo de implementación. Prepararla
  tras publicar B03: normalización del mando manual, independiente del ADC físico.
- main sigue mínimo; servicios no integrados al hardware. Hooks/firma conservados.

## Evidencia aceptada B03

| Verificación | Resultado |
|---|---|
| CTest host / sanitizadores | 49 casos: B01 17 + B02 17 + B03 15; sin hallazgos ASan/LeakSanitizer/UBSan |
| Aritmética B03 | 100 fronteras + 10 000 ventanas; redondeo, rango y recuperación |
| ARM Debug / Release | CTest Debug 5/5 (B00 14 casos); fixture B03 Release correcto |
| Doxygen estricto | Sin avisos; 15 funciones/USW totales, incluidas 2 nuevas B03 |
| Recursos estáticos B03 | Lectura 8 bytes; código -O0 236 bytes; stack interno hasta 128 bytes con libgcc |
| ELF | Sin indefinidos; GNU_STACK RW; control negativo rechaza pila ejecutable |
| Cierre documental | Sin cambios de firmware; requisitos idénticos y archivos anteriores preservados |

Matriz S1–S7 y comandos en verify-report.md archivado; aceptación y manifest en
archive-report.md. Logs implementación: /tmp/ed3-b03-20261106-07i2qp2_.
Logs cierre: /tmp/ed3-cierre-b03-20261106-zpksl5sb (temporales, fuera de Git).
No se repiten pruebas de firmware en el cierre porque no cambian fuentes ni build.
Sin ejecución ARM, simulación ni prueba física; no se flasheó ni instaló nada.

## Decisiones y limitaciones

- B03: 60 pulsos/vuelta, conteo/duración us uint32_t; salida milésimas de RPM,
  cercano/empate arriba y estados explícitos. Las lecturas pueden superar 185 RPM.
- Captura coherente, serialización y overflow previo requieren integración física;
  sin ventana física fijada ni timeout de frescura. Runtime libgcc existente, sin libc.
- R15: aplicación portable entre configuraciones físicas; abstracciones y explicación
  de decisiones en lenguaje sencillo registradas en CONTEXTO.
- Consignas siguen 0..185; orden global/FIFO B01/B02 conservados.
- Faltan captura, UART real, telemetría, PWM/control, ADC/DMA, alarmas y main;
  pines, revisión PCB/silicio, VREF y reloj pendientes. CMSIS-DAP integrado confirmado.
- Sin CI ni aceptación física. Doxyfile parcial; documentación final aún pendiente.
- AGENTS/.atl locales por ignore global; procedimiento compartido en docs/agente/.

## Próxima acción

Publicar commits B03 autorizados y preparar propuesta B04 de normalización del mando
manual (0..1000, fondo de escala explícito, errores observables). Acordar alcance y
aceptación antes de implementar B04. No extender autorización de push a bloques futuros.

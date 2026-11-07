# Estado — 2026-11-06

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama main; base B03: 27ad99a7d04c52dfd98344857fa5c98ed7ad61d4.
  Revisión al reanudar: `git rev-parse HEAD`; sincronización: `git status -sb`.
- B00–B02 aceptados/archivados; contratos build/modos/comunicación vigentes.
- B03 acordado, implementado y verificado; resultado pendiente de aceptación.
  Carpeta activa: openspec/changes/b03-medicion-velocidad/.
- Cierre local con asunto `feat: implementar B03 de medicion de velocidad`;
  consultar `git log -1` para hash. Commit autorizado; push explícitamente no autorizado.
- Specs B03 candidatas: sin promoción ni archivo hasta aceptación del resultado.
- main sigue mínimo; servicios no integrados al hardware. Hooks/firma conservados.

## Evidencia ejecutada B03

| Verificación | Resultado |
|---|---|
| CTest host | 3/3; 17 B01 + 17 B02 + 15 B03 = 49 casos |
| ASan/LeakSanitizer/UBSan | 49 casos sin hallazgos, fuera del sandbox/ptrace |
| Límites aritméticos B03 | 100 fronteras + 10 000 ventanas, redondeo y rango; prueba final B03 15/15 normal/san |
| ARM Debug CTest | 5/5; B00 14/14 y fixtures B01/B02/B03 |
| ARM Release | Build y fixture B03 a -O3 correctos; API y división uint64 retenidas |
| Doxygen estricto HTML/XML | Sin avisos; 2 USW B03 nuevas, 15 funciones totales |
| Recursos B03 -O0 | Lectura 8 bytes, código 236 bytes sin global mutable; stack interno hasta 128 bytes con libgcc |
| ELF | Sin indefinidos; GNU_STACK RW; control negativo rechaza pila ejecutable |
| Preservación/diff | Archivos aceptados, main, B01/B02 y drivers intactos; diff revisado |

Informe completo: openspec/changes/b03-medicion-velocidad/verify-report.md.
Logs: /tmp/ed3-b03-20261106-07i2qp2_ (temporales, fuera del árbol versionado).
Primer enlace rechazado por nota GNU-stack faltante en libgcc: se declaró pila
no ejecutable en ELF y se comprobó, manteniendo avisos como errores. Doxygen
detectó delimitador de grupo inválido: corregido y generación completa verificada.
No hay fallos pendientes del alcance. No se ejecutó ARM en MCU/simulador, no se
flasheó, instaló nada ni cambió configuración del microcontrolador.

## Decisiones y limitaciones

- B03: pulsos/ventana en us uint32_t, 60 pulsos/vuelta, milésimas de RPM uint32_t;
  cercano/empate arriba. Lectura no recortada a 185; estados SIN_DATOS/VALIDA/error.
- Duración cero o resultado redondeado excesivo invalidan y borran valor previo;
  captura coherente, serialización y overflow previo son responsabilidad del integrador.
- Runtime libgcc existente enlazado sin libc. Pila ELF RW no configura MPU físico.
- Consignas siguen 0..185 inclusive; B01/B02 mantienen contrato y orden global/FIFO.
- Faltan captura/sensor, UART real, telemetría, PWM/control, ADC/DMA, alarmas y main.
  Pines, revisión PCB/silicio, VREF y reloj pendientes. CMSIS-DAP integrado confirmado.
- No hay CI ni aceptación física. No consta aprobación formal del anteproyecto aquí.
- Doxyfile parcial: main, Procesamiento, Comunicacion, Medicion_Velocidad y límites.
  Documentación final de todos los módulos aún pendiente.
- AGENTS/.atl locales por ignore global; procedimiento compartido en docs/agente/.

## Próxima acción

Revisar aceptación del resultado B03 con el usuario. Sólo tras ella y orden de cierre,
sincronizar specs y archivar. No iniciar B04 ni hacer push sin autorización.

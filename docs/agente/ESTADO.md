# Estado — 2026-11-07

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama main; B05 inicial fcd43927d7b2dbaed19169d4cfb249de4a3da16f local.
  Base remota conocida: 8d5bc7b040fc205af48a00db35de5f7622c82379.
  Verificar HEAD y sincronización con `git rev-parse HEAD` / `git status -sb`.
- B00–B04 aceptados/archivados; no editar historial ni contratos aceptados.
- B05 corregido por aclaración R15: servicio genérico y conversión específica en LM35.
  Resultado verificado, pendiente de aceptación. Cambio activo b05-conversion-temperatura.
- Commit correctivo con asunto `fix: desacoplar Temperatura del sensor LM35`;
  consultar `git log -1` para hash. Historial conservado; sin push autorizado.
- verify-report-inicial.md preserva evidencia de fcd4392; verify-report.md actual
  acredita separación nueva. Specs candidatas sin promoción ni archivo.
- main mínimo, sin integración de adquisición ni aceptación física. Hooks/firma conservados.

## Evidencia actual ejecutada

| Verificación | Resultado |
|---|---|
| Reproducción / corrección | Mismo consumidor Celsius/estado: antes no compilaba, ahora 10/10 sin LM35 |
| Host / sanitizadores | CTest 7/7; 64 B01–B04 + 10 servicio + 17 LM35 + 3 integración = 94 casos sin hallazgos |
| ARM Debug / Release | 8/8 Debug (B00 14 casos), 2/2 fixtures B05 Release |
| Doxygen estricto | Sin avisos; 4 USW B05 entre servicio/dispositivo, 21 funciones totales |
| Servicio -O0 | Código 142 bytes, lectura 8; stack propio hasta 24; sin auxiliares externos |
| LM35 -O0 | Código 390 bytes, lectura 8; stack interno hasta 176 con libgcc |
| Separación | Temperatura sin ADC/modelo/ganancia/LM35; dispositivo sin dependencia del servicio; adaptación probada |
| Preservación | Cálculo previo idéntico en LM35; B00–B04, drivers ajenos y archivos aceptados intactos |

Matriz S1–S8 y comandos: verify-report.md del cambio activo.
Logs: /tmp/ed3-b05-desacople-20261107-ltmr1lup (temporales, fuera de Git).
Sin fallos pendientes del alcance; sin simulación, ejecución MCU ni prueba física.
No se flasheó, instaló nada ni cambió configuración del microcontrolador.

## Decisiones y pendientes

- Servicios abstraen hardware específico (aclaración del usuario); Temperatura
  recibe int32_t centésimas y fuente válida/sin datos/error, sin parámetros eléctricos.
- LM35 transforma ADC a Celsius con parámetros de escala/referencia y errores propios;
  conversión nominal sin offset, todavía sin adquisición. Datasheet local en CONTEXTO.
- Adaptación de errores/rango uint32→int32 probada en test/fixture. Composición con
  adquisición real pendiente; no introducir acceso a dispositivos desde lógica de aplicación.
- B03 mantiene 60 pulsos/vuelta fijos por contrato aceptado: revisión transversal
  pendiente bajo R15, fuera de esta corrección. No afirmar desacoplamiento total aún.
- Siguen pendientes ADC/DMA, pines/VREF/montaje, selector, captura, UART/telemetría,
  PWM/control, alarma, reloj y main. CMSIS-DAP integrado; revisión PCB/silicio pendiente.
- AGENTS/.atl locales por ignore global; procedimiento compartido en docs/agente/.

## Próxima acción

Revisar aceptación del B05 corregido; sólo después sincronizar/archivar por orden.
No hacer push ni iniciar otro bloque sin autorización.

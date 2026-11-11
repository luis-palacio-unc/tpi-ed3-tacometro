# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  Base b473d0f, publicado62eef53; consultar HEAD/origin/main al reanudar.
  Tres commits de B15 más cierre/propuesta locales, sin push autorizado.
- B00–B15/B03-R1/B07-R1 aceptados/archivados. Contrato motor-dispositivo sincronizado.
- Activo B16 b16-lectura-potenciometro-dma: propuesta sin acuerdo/código/pruebas.
  [Propuesta](../../openspec/changes/b16-lectura-potenciometro-dma/proposal.md).
- B15: Motor_Write valida0..1000 y envía el valor exacto a PWM_MATCH_1 por driver,
  incluido1000. MR0=1000 se configurará al integrar. Sin cálculo/lectura propia.
  PWM.1/frecuencia/arranque/potencia/servicio/main pendientes, según archivo aceptado.
- Drivers90 originales a21cc1e intocables. Inicialización/enlace para el final.
  No iniciar B16 sin acuerdo ni retomar B12 diferido.

## Evidencia aceptada B15

Host/san21/21,204 casos C+oráculo Python; cuatro casos B15,1001 mandos válidos y
64535 rechazos. ARM13/13 y objetoO0/O2/header; Doxygen42 sin avisos. Único externo
PWM_MatchUpdateSingle. Recursos propios .text42/24,marco16/8 bytes,datos0; no cota total.
Biblioteca/módulos/contratos aceptados intactos; firmware18 objetos/main mínimo.
[Informe archivado](../../openspec/changes/archive/2026-11-10-b15-dispositivo-motor-pwm/verify-report.md).
Logs B15: /tmp/ed3-b15-directo-2yz3lskk. Cierre documental: /tmp/ed3-cierre-b15-propuesta-b16-zasor5qr.
No se repitieron pruebas de firmware al archivar; sin ejecución MCU/placa/publicación.

## Próxima acción y límites B16

Acordar canal0/P0.23 y lectura del potenciómetro antes de implementar. Pot_Read
propuesto recibe dato DMA por valor y salida uint16_t; ADC_Decode común usa macros
del driver para RESULT/CH/DONE/OVERRUN. Estados VALIDA/SIN_DATOS/ARGUMENTO/
OVERRUN/CANAL, sólo consumir VALIDA; salida escribible no válida queda cero.
Header propio sin CMSIS; ESW12/USW12.1 y ESW18 local/USW18.1. Mandos/LM35 intactos.
Bloque DMA debe estar completo/estable, no en escritura concurrente. Configurar
fuente/secuencia/ADC.2 y buffer/canales/IRQ al final; no prometer frescura/ADC real.
Canal/tag y muestra se validan sin normalizar; composición Mandos sólo fixture.
Quedan adquisición ADC/DMA física, enlace de LM35, regulación e integración final.
Diagramas del informe al terminar todos los ESW.

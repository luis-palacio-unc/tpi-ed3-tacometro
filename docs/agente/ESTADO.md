# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`,
  main, consultar HEAD. Publicado cb380bc;3adfc6a y cierres locales, sin push.
- B00–B13/B03-R1/B07-R1 aceptados/archivados. B13 cerrado2026-11-10:
  contrato uart-dispositivo vigente, archivo2026-11-10-b13-dispositivo-uart.
- UART0: Read/Write no bloqueantes, conteo de bytes, TX P0.2/RX P0.3 asignados;
  UART_BAUDRATE default115200 configurable. Parámetro/pinmux aún sin aplicar.
- Activo propuesto b14-sensor-optico-contador (B14), sin acuerdo/código/pruebas:
  SensorOptico_Read devuelve eventos acumulados por TIM_ReadTimer(LPC_TIM1).
  Propuestos TIMER1/CAP1.0/P1.18 función11, flanco ascendente; TIMER0 base intacta.
- Integración/ventanas, configuración/arranque de periféricos y uso desde servicios/
  aplicación/main explícitamente para el final. No modificar Drivers,90 originales
  conservados. Sin push autorizado. No iniciar B14 sin acuerdo ni retomar B12 diferido.

## Evidencia aceptada y cierre actual

B13 3adfc6a: host/san19/19,194 casos C y oráculo Python;11 casos UART. ARM13/13 y
objetosO0/O2, Doxygen40 sin avisos; baud overrides válidos/rechazo inválidos, negativos
de conteo/modo detectados. Drivers/servicios/módulos aceptados intactos; sin ensayo físico.
[Informe](../../openspec/changes/archive/2026-11-10-b13-dispositivo-uart/verify-report.md).

Cierre sólo documental: igualdad UD1–UD4/S1–S7, estructura/enlaces/hashes y diff.
No repetir pruebas de firmware ni atribuirlas a B14. Manual/driver B14 sólo leídos;
pin y CTCR revisados, renders en /tmp/ed3-b14-manual-1kmbsz63.
Logs de cierre: /tmp/ed3-cierre-b13-propuesta-b14-inendql4.

## Próxima acción y pendientes

Acordar [B14](../../openspec/changes/b14-sensor-optico-contador/proposal.md), incluido
TIMER1/CAP1.0/P1.18, antes de implementar. Una USW.2.1.10.1 prevista. Conteo puro,
sin reinicio/ventana/RPM/contexto/IRQ/estado local. Prueba de composición B03 en host.
Precondiciones futuras: contador exclusivo/configurado/arrancado y señal compatible;
CCR del canal000, sin matches que reinicien/detengan. Nueva época tras reset exige
nueva referencia; cadencia menor que2^32 eventos por intervalo. No hay detección
de vueltas completas omitidas. Quedan Motor, Potenciometro, ADC-DMA/LM35,
regulación e integración física; diagramas del informe al completar todos los ESW.

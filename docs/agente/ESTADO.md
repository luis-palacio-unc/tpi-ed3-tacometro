# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`,
  main, consultar HEAD. Base7409bee; publicado cb380bc; commits locales sin push.
- B00–B13/B03-R1/B07-R1 aceptados/archivados. Activo B14 b14-sensor-optico-contador:
  alcance/TIMER1/CAP1.0/P1.18 autorizados2026-11-10, implementado/verificado, sin aceptar.
- SensorOptico_Read devuelve eventos acumulados uint32_t con TIM_ReadTimer(LPC_TIM1),
  sin reset/escala/RPM/ventana/IRQ/contexto/estado propio. USW.2.1.10.1 documentada.
- XD-51 confirmado por usuario, sustituye HC-89 del anteproyecto; no deducir niveles/
  pinout/alimentación. Señal y configuración de contador/pin para integración final.
- Drivers90 originales a21cc1e, Timer/B03/otros servicios/dispositivos/main y archivos/
  contratos aceptados intactos. Commit local al terminar; sin push/sincronizar/archivar
  hasta aceptación/orden correspondientes. No iniciar otro bloque ni integrar ahora.

## Verificación B14 realmente ejecutada

| Modalidad | Resultado |
|---|---|
| Host/san |20/20 cada uno,200 casos C+oráculo Python; sin hallazgos |
| B14 individual |6 casos: conteos/repetición/avance/ventanasB03/rollover/referencia externa/factor |
| Regresiones |11199 ternas velocidad,5096 pares codec,4096 muestras/400 vueltas selector |
| ARM |Build/controles13/13, móduloO0/O2 y headerC11/C++17; sin ejecución MCU |
| Doxygen |41 funciones únicas HTML/XML sin avisos,1 nueva |
| Negativos externos |Detectan TIMER0 equivocado, escala temporal y recorte185 |
| Recursos propios |.text20-O0/12-O2,.data/.bss0,marco8/0 bytes sin driver/caller/IRQ |
| Preservación |261 archivos protegidos intactos,90 Drivers originales; firmware sigue18 objetos |

[Informe](../../openspec/changes/b14-sensor-optico-contador/verify-report.md).
Logs externos: /tmp/ed3-b14-optico-ibtde439. El doble sólo proporciona conteos de API; no simula CPU/señal
ni ejecuta biblioteca/contador físico. Sin hardware/flasheo/instalación/publicación.

## Próxima acción y límites

Revisar/aceptar B14 antes de sincronizar/archivar. Contador exclusivo preconfigurado/
arrancado por CAP1.0/P1.18 función11, flanco ascendente; CCR del canal000, sin match
que reinicie/detenga y acceso serializado. Resta externa de misma época con menos
de2^32 eventos por intervalo; nueva referencia tras reset, no detectar vueltas completas.
PinConfig/InitCounter/Enable, señal/cableado XD-51, ventanas y servicios/aplicación/
main quedan para integración final. Quedan Motor, Potenciometro, ADC-DMA/LM35 y
regulación. No retomar B12 diferido; diagramas del informe al terminar todos los ESW.

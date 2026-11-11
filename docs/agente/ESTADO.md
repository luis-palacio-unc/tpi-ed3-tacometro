# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  Base c952892; publicado62eef53. Consultar HEAD/Git al reanudar, sin push autorizado.
- B00–B14/B03-R1/B07-R1 aceptados/archivados. Activo B15 corregido por usuario,
  implementado/verificado, resultado sin aceptar; no sincronizar ni archivar aún.
- API: bool Motor_Write(uint16_t mando_por_mil). Sólo mando0..1000; lee una vez
  LPC_PWM1->MR0 a const local. Sustituye período explícito de c952892.
  Rechaza MR0=0/MAX; redondeo cercano/empate arriba; MR1=0 bajo y MR1=MR0+1
  alto continuo al redondear al completo. Escritura por PWM_MatchUpdateSingle original.
- Sin contexto/caché/estado propio/init/ISR/esperas. Doxygen USW.2.1.9.1 corregido.
  Modelo RAM de PWM sólo en pruebas_motor, con tipos CMSIS originales; firmware/
  ARM usan dirección real. No modificar Drivers (90 originales a21cc1e).
- Commit local de corrección al terminar; sin push/merge/reescritura de historia.
  Servicios/otros dispositivos/contratos y archivos aceptados conservados.

## Evidencia actual realmente ejecutada

| Modalidad | Resultado |
|---|---|
| Host/san | 21/21 cada uno,206 casos C+oráculo Python; sin hallazgos |
| B15 | Seis casos;13013 combinaciones/64535 rechazos; cambio MR0 entre llamadas |
| Regresiones | 11199 ternas velocidad,5096 pares codec,4096 muestras/400 rollovers selector |
| ARM | Build/controles13/13, MotorO0/O2/headerC11/C++17, lectura MR0 real inspeccionada |
| Doxygen | 42 funciones HTML/XML sin avisos; Motor_Write con un parámetro |
| Negativos | Detectan MR1 equivocado/período fijo, canal/producto32/redondeo/extremo alto |
| Recursos propios | .text168-O0/72-O2,.data/.bss0,marco64/8 bytes; no cota total |
| Preservación | 270 archivos protegidos,90 Drivers originales,firmware18 objetos |

[Informe actual](../../openspec/changes/b15-dispositivo-motor-pwm/verify-report.md).
Logs: /tmp/ed3-b15-mr0-y8gcpwyc. Evidencia anterior de c952892 histórica, no prueba de la firma actual.
Modelo de registro/API sólo en PC; no biblioteca/MMIO/CPU/latch/motor físicos.
Sin ejecución ARM/placa, simulación MCU, instalaciones, flasheo o publicación.

## Próxima acción y límites

Revisar/aceptar B15 corregido antes de sincronizar/archivar. MR0 leído debe ser
efectivo/estable sin update pendiente; leerlo no prueba que pasó al latch activo.
PWM exclusivo/accesible/preconfigurado/arrancado, PWM1.1/P2.0 función01 single-edge
alto, reset por MR0 y sin otros matches que alteren TC; acceso serializado.
Frecuencia/configuración/potencia/adaptación B08/main y ensayos para el final.
TIMER0/base y TIMER1/CAP1.0/P1.18 del XD-51 conservados; señal/ventana1s pendientes.
Quedan Potenciometro, ADC-DMA/LM35, regulación e integración física final.
No avanzar otro bloque ni retomar B12 diferido; diagramas al terminar todos los ESW.

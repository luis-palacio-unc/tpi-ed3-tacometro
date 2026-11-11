# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  Base/publicado62eef53; consultar HEAD. Commit B15 local al terminar, sin push.
- B00–B14/B03-R1/B07-R1 aceptados/archivados. Activo B15 b15-dispositivo-motor-pwm,
  alcance/PWM1.1/P2.0 autorizados2026-11-10, implementado/verificado, sin aceptar.
- Motor_Write(mando_por_mil,periodo_ticks): mando0..1000, período1..UINT32_MAX-1,
  redondea cercano/empate arriba y delega PWM_MATCH_1. MR1=0 pide bajo, MR1=período+1 pide alto
  continuo al redondear al completo. false rechaza sin llamada; true es despacho.
  Sin contexto/estado propio/init/ISR/esperas. USW.2.1.9.1 documentada con Doxygen.
- Drivers90 originales a21cc1e, Driver_Motor/otros servicios/dispositivos/main,
  contratos y archivos aceptados intactos. No sincronizar/archivar sin aceptación.
  No iniciar otro bloque ni configurar/integrar periféricos ahora.

## Verificación B15 realmente ejecutada

| Modalidad | Resultado |
|---|---|
| Host/sanitizadores | 21/21 cada uno,206 casos C+oráculo Python; sin hallazgos |
| B15 individual | Seis casos,13013 combinaciones por cota/monotonía y64535 rechazos |
| Regresiones | 11199 ternas velocidad,5096 pares codec,4096 muestras/400 vueltas selector |
| ARM | Build/controles13/13, MotorO0/O2/headerC11/C++17; sin ejecución MCU |
| Doxygen | 42 funciones únicas HTML/XML sin avisos, una nueva |
| Negativos externos | Detectan canal equivocado,producto32,truncamiento y extremo alto |
| Recursos propios | .text156-O0/64-O2,.data/.bss0,marco56/8 bytes; no cota total |
| Preservación | 270 archivos protegidos intactos,90 Drivers originales; firmware18 objetos |

[Informe](../../openspec/changes/b15-dispositivo-motor-pwm/verify-report.md).
Logs externos: /tmp/ed3-b15-motor-e9u__4fp. Símbolos/helpers/relocaciones y dos ajustes a auditorías
externas registrados en informe; no cambios de firmware para obtener verde.
Doble de API de match, sin ejecución de biblioteca/MMIO/CPU/PWM o motor real.
Sin flasheo, instalaciones o publicación.

## Próxima acción y límites

Revisar/aceptar B15 antes de sincronizar/archivar. Uso físico exige PWM exclusivo,
single-edge activo alto en PWM1.1/P2.0 función01; período igual a MR0 efectivo/
estable, sin actualización pendiente, reset sólo por MR0 y acceso serializado.
Latch en próximo ciclo; varias órdenes pueden dejar sólo la última. Frecuencia,
configuración/arranque, etapa de potencia, adaptación B08/main y ensayo al final.
TIMER0/base y TIMER1/CAP1.0/P1.18 del XD-51 conservados; señal/ventana1s pendientes.
Quedan Potenciometro, adquisición ADC-DMA/LM35, regulación e integración física final.
No retomar B12 diferido; diagramas del informe al terminar todos los ESW.

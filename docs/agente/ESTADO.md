# Estado — 2026-11-10

## Punto actual

- Repo `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`, main.
  Punto de partida 3fa08ea; consultar HEAD y origin/main al reanudar.
- B00–B14/B03-R1/B07-R1 aceptados y archivados; sensor-optico sincronizado.
  Usuario autorizó publicar los ocho commits pendientes y este cierre/propuesta.
  Git registra el resultado de publicación; no inferir permisos futuros ni merge.
- Activo: B15 b15-dispositivo-motor-pwm, propuesta sin acuerdo/implementación/pruebas.
  PWM1.1/P2.0 función 01 propuestos; firma y aceptación en
  [propuesta](../../openspec/changes/b15-dispositivo-motor-pwm/proposal.md).
- B14: SensorOptico_Read devuelve conteo uint32_t por TIM_ReadTimer(LPC_TIM1).
  TIMER1/CAP1.0/P1.18 para contador externo ascendente; no captura de timestamps.
  XD-51 actual confirmado; reemplaza HC-89 del anteproyecto. Electricidad pendiente.
- No modificar Drivers: 90 originales a21cc1e. Integración/configuración/enlace de
  periféricos al final. No iniciar B15 sin acuerdo ni retomar B12 diferido.

## Evidencia aceptada B14

Host/sanitizadores 20/20, 200 casos C más oráculo Python; seis casos B14.
Regresiones: 11199 ternas velocidad, 5096 pares codec, 4096 muestras/400 rollovers
selector. ARM 13/13 y módulo O0/O2/header C11/C++17; Doxygen 41 funciones sin avisos.
Negativos externos detectan TIMER0 equivocado, escala temporal y recorte 185.
.text propio 20/12 bytes y marco 8/0 O0/O2; sin datos/estado propios, no cota total.
261 archivos protegidos intactos; fuente firmware sigue 18 objetos, main mínimo.
[Informe](../../openspec/changes/archive/2026-11-10-b14-sensor-optico-contador/verify-report.md).
Logs B14: /tmp/ed3-b14-optico-ibtde439. Cierre documental: /tmp/ed3-cierre-b14-propuesta-b15-uejb662y.
No se repitieron pruebas de firmware al archivar; no ejecución MCU/placa ni simulación.

## Próxima acción y límites

Acordar B15 antes de implementar. Motor_Write propuesto recibe mando 0..1000 y
período 1..UINT32_MAX-1 en ticks; convierte/redondea, rechaza inválidos sin llamada
y delega sólo match de salida en driver PWM. Sin contexto/estado propio/init.
Período igual a MR0 activo/estable; frecuencia/potencia/adaptación B08/main al final.

Para usar B14 físicamente: TIMER1 exclusivo/configurado/arrancado, CAP1.0/P1.18,
CCR del canal 000, sin matches que reinicien/detengan y señal compatible.
Referencias de misma época y menos de 2^32 eventos; nueva referencia tras reset.
PinConfig/InitCounter/Enable, ventana de 1 s y conexión a servicios/main pendientes.
Después quedan Potenciometro, ADC-DMA/LM35, regulación e integración física final.
Diagramas del informe al terminar todos los ESW.

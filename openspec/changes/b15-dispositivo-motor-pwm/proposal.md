# B15 — Actualización PWM del dispositivo Motor

Estado: alcance/PWM1.1/P2.0 acordados2026-11-10; implementación autorizada, resultado sin aceptar.

## Objetivo y origen

R03/R06/R10/R11/R15 y ESW.2.1.9 del anexo B: traducir el mando lógico del motor
a una actualización PWM usando la biblioteca original. Continúa dispositivos;
inicialización y conexión a Driver_Motor/main se dejan para el final.

## Alcance

- PWM1.1 acordado en P2.0, función 01 de PINSEL4, salida single-edge activa alta.
  P1.18 permanece CAP1.0/TIMER1 del sensor, TIMER0 conserva la base temporal.
- Motor_Write(uint16_t mando_por_mil, uint32_t periodo_ticks) devuelve bool.
  Mando 0..1000 y período 1..UINT32_MAX-1, igual al MR0 activo y estable.
  Período explícito evita estado global y no impone frecuencia/reloj nuevos.
- Convertir a ticks con redondeo cercano/empate arriba, error máximo medio tick.
  Cero produce salida baja; redondeo a período completo usa MR1=período+1 para
  salida alta continua, según manual. Una llamada PWM_MatchUpdateSingle por orden.
- Rechazar mando >1000 o período 0/UINT32_MAX con false y sin llamadas.
  true significa solicitud válida enviada al driver, sin confirmar electricidad.
  Sin contexto opaco ni fallos de hardware inventados.
- Doxygen ESW.2.1.9/USW.2.1.9.1 y pruebas verificables de conversión/despacho.

Archivos: Dispositivos/Motor.h/.c, tests/host/test_motor.c, CMakeLists.txt,
docs/doxygen/Doxyfile y seguimiento. Drivers, Timer, Sensor_Optico, Driver_Motor,
aplicación y contratos aceptados se preservan.

Excluye init/arranque/pinmux/reloj, fijar frecuencia, potencia/dirección/freno,
regulador/rampas, adaptación del servicio y ensayos físicos.

## Dependencias y aceptación

UM10360 tabla 84/p120, §24.4.1/p524 y §24.6.7/p531; driver PWM existente.
Uso físico exige PWM exclusivo, MR0 coherente, modo/reloj/canal habilitados y
serialización; efecto diferido al próximo ciclo, no espera ni confirmación física.
Frecuencia y etapa de potencia se definirán en integración final.

1. Verificar 0/500/1000 con período 1000: MR1=0/500/1001, sin alterar período.
2. Verificar redondeo, repetición, fronteras y períodos pequeños/grandes sin overflow.
3. Entradas inválidas no escriben; una válida posterior recupera sin reintento.
4. Host/sanitizadores/regresiones, ARM y Doxygen pasan; biblioteca intacta.

Riesgo: confundir porcentaje/ticks o asumir salida inmediata. Contratos y casos
numéricos lo detectan; cableado/señal se verifican al integrar. Reversión: commit
inverso sólo del bloque acordado. Implementar únicamente tras aceptar esta propuesta.

# B15 — Motor con período fijo y mando directo

Estado: simplificación autorizada por usuario2026-11-10, resultado sin aceptar.

## Objetivo y origen

R03/R06/R10/R11/R15, ESW.2.1.9: actualizar PWM mediante biblioteca existente.
El usuario fija MR0=1000; cada unidad de mando pasa a ser directamente un tick.
Se reemplazan la conversión genérica y lectura de MR0 de versiones anteriores.

## Alcance y contrato

- bool Motor_Write(uint16_t mando_por_mil), válido0..1000.
- Una llamada PWM_MatchUpdateSingle(PWM_MATCH_1,mando_por_mil) por orden válida.
  Sin cálculo de match, lectura MR0, producto/división/redondeo o1000→1001.
- Rechazo >1000 con false, sin llamadas/cero automático; true indica despacho.
- MR0=1000 ticks efectivo/estable será condición de inicialización final, usando
  drivers originales. No configurar ahora ni escribir MR0 por llamada.
- PWM1.1/P2.0 función01 conservado. Documentar USW9.1 y probar nuevo contrato.

Archivos: Motor.h/.c, test_motor.c, CMakeLists.txt y documentación/SDD.
Retirar modelo de registros/preinclude propios de la prueba anterior.
No modificar Drivers, Timer, Sensor_Optico, servicios/otros dispositivos,
contratos aceptados o archivos archivados. Sin regulación/rampas/init/clock/
potencia/dirección/freno/main ni ensayos físicos nuevos.

## Dependencias y aceptación

MR0=1000, PWM accesible/exclusivo/configurado/arrancado/single-edge alto,
reset por MR0 sin otros matches que alteren TC; acceso serializado/latch.
Frecuencia/prescaler, potencia y validación PWM.1 al integrar; ver exploración.

1. Mandos0/250/500/1000 despachan exactamente esos valores, también repetidos.
2. Recorrer0..1000 sin conversiones y1001..65535 sin despacho, con recuperación.
3. Host/san/regresiones, ARM/header y Doxygen pasan, sin helpers de conversión.
4. Biblioteca y módulos aceptados intactos; hardware/arranque permanecen pendientes.

Riesgo: asumir equivalencia sin configurar MR0=1000 o ignorar PWM.1; documentar
condición y ensayo físico final. Reversión por commit inverso sólo del cambio.
Commit local, sin push; aceptar resultado antes de sincronizar/archivar.

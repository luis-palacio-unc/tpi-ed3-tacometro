# Diseño candidato B15 — mando con lectura de MR0

## Cambio acordado y decisiones

El usuario pide simplificar B15: Motor_Write recibe sólo mando y consulta MR0.
Esta decisión reemplaza el período explícito de c952892, cuyo código/informe
se conservan en Git como evidencia anterior. No se modifica ningún driver.

| Elección | Razón |
|---|---|
| bool Motor_Write(uint16_t mando_por_mil) | El llamador sólo suministra mando; no duplica configuración del PWM. |
| Una lectura volatile de LPC_PWM1->MR0 por llamada válida | Dispositivos es específico del MCU; no existe getter en biblioteca. Sin caché ni wrapper adicional. |
| Escritura con PWM_MatchUpdateSingle(PWM_MATCH_1, valor) | Reutiliza actualización de MR1/LER original, sin copiarla. |
| PWM1.1/P2.0 función01 single-edge | Recurso acordado; conserva P1.18 para CAP1.0/TIMER1 y TIMER0 para base temporal. |
| Modelo RAM sólo en pruebas host | Probar código real y tipos CMSIS originales sin acceso a MMIO físico ni cambios a biblioteca. |

## Operación, aritmética y precondiciones

Validar mando <=TPI_MANDO_MANUAL_MAX (1000 compartido). Leer MR0 una vez a una
variable local const. Rechazar MR0=0/UINT32_MAX sin despacho. Mantener conversión:
q=round_half_up(mando*período/1000), intermedio uint64_t cuyo máximo es
1000*(2^32-2)+500. Si q<período, MR1=q; si q=período, MR1=período+1 para alto
continuo. Reservar UINT32_MAX evita overflow de ese extremo; error <=medio tick.
Una llamada al driver por orden válida, incluso repetida. true confirma despacho;
false rechaza sin emitir cero. Sin estado/contexto/heap/ISR/esperas/init propios.

La lectura obtiene MR0 programado; no acredita que una actualización pendiente
ya se haya aplicado por latch. Por eso el uso físico exige PWM accesible,
preconfigurado/arrancado/exclusivo y MR0 efectivo/estable sin update pendiente.
Reset por MR0, sin otros matches que alteren TC, acceso privilegiado serializado.
Varias órdenes antes del próximo ciclo pueden dejar sólo la última. Frecuencia,
reloj/pinmux/arranque y etapa de potencia para integración final.
Manual UM10360 §24.4.1/p524 y §24.6.7/p531 conserva fundamento de extremos/latch.

## Pruebas y archivos

- Motor.h/.c: firma de un parámetro, lectura local y Doxygen USW9.1 actualizado.
- tests/host/dobles/pwm_registros.h: incluir LPC17xx.h original, declarar estructura
  RAM con su tipo y redirigir únicamente LPC_PWM1 en compilación de pruebas.
- CMakeLists.txt: preinclude privado sólo en pruebas_motor; no afecta firmware,
  otros targets ni archivos en Drivers. Header público sigue sin CMSIS.
- test_motor.c: MR0 del modelo provee período; doble de escritura conserva
  despacho/rango. S4/S6 cambian MR0 entre llamadas con el mismo mando y preservan
  MR0/otros campos. S3 configura MR0 inválido y prueba recuperación. Mantener
  tablas/límites, 13013 combinaciones y64535 rechazos, sin copiar fórmula en pruebas.
- README/docs/agente/docs-doxygen y seguimiento SDD: contrato/evidencia actuales.

Ejecutar host/sanitizadores/regresiones, ARM/header O0/O2 sin redirección, Doxygen
estricto, recursos/símbolos y hashes. Negativos externos comprueban lectura de MR1
equivocada/período fijo, además de fallos numéricos. Header sólo C11/C++17.
Sin ejecutar biblioteca/MCU, reloj/señal/motor físicos; modelo de RAM no simula
latch, tiempo o electricidad. Driver_Motor conserva su contrato, adaptación al final.
El resultado B15 corregido requiere aceptación antes de sincronizar/archivar.

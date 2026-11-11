# Dispositivo Motor — candidato B15 simplificado

Origen R03/R06/R10/R11/R15. El usuario fija MR0=1000 y exige envío directo del
mando2026-11-10. Sustituye conversión/lectura de cf17359; resultado sin aceptar.

## Requirements

### Requirement: MD1 — Período fijo y mando directo

Motor MUST exponer bool Motor_Write(uint16_t mando_por_mil). Mando válido MUST
ser0..1000 y enviarse íntegro a PWM_MATCH_1 con PWM_MatchUpdateSingle original.
MUST NOT calcular match, escalar, redondear, leer MR0 o transformar1000 en1001.
La configuración de uso MUST fijar MR0=1000 ticks en la inicialización final;
Motor_Write MUST NOT reconfigurar el período por llamada. No interpretar RPM.

#### Scenario: S1 — Valores de referencia
- Given PWM preconfigurado con MR0=1000 y mandos0/250/500/1000.
- When se solicita cada mando.
- Then MUST despachar valores0/250/500/1000 exactos y devolver true.

#### Scenario: S2 — Dominio completo
- Given cada mando representable entre0 y1000.
- When se llama Motor_Write.
- Then MUST enviar el mismo valor una vez, sin recorte185 ni caso especial del máximo.

### Requirement: MD2 — Rechazo y recuperación

MUST devolver false si mando>1000, sin llamar al driver ni emitir cero.
Una llamada válida posterior MUST recuperar sin retener errores. true MUST
significar despacho lógico, no confirmar estado eléctrico o movimiento.
La configuración MR0=1000 MUST ser precondición externa, sin diagnóstico nuevo.

#### Scenario: S3 — Fuera de rango y recuperación
- Given orden válida y mandos1001..UINT16_MAX posteriores.
- When se rechazan y luego llega otra orden válida.
- Then MUST evitar toda escritura por rechazo y recuperar con un único despacho.

### Requirement: MD3 — Operación mínima de dispositivo

MUST conservar PWM1.1/P2.0 función01, un despacho por orden válida aun repetida,
sin estado/contexto/caché/init/reset/ISR/heap/esperas ni acceso propio a registros.
Uso físico MUST exigir PWM accesible/exclusivo/preconfigurado/arrancado,
single-edge activo alto, MR0 efectivo1000 estable, reset por MR0 y sin otros
matches que alteren TC. Acceso serializado y latch al próximo ciclo.

#### Scenario: S4 — Repeticiones y cero
- Given órdenes repetidas, cero, máximo y descenso posterior.
- When se aplica la secuencia.
- Then MUST despachar cada valor una vez y preservar período/otros recursos.

### Requirement: MD4 — Documentación y evidencia

MUST actualizar Doxygen ESW.2.1.9/USW.2.1.9.1 y verificar host/san/ARM/regresiones.
MUST retirar el modelo RAM de MR0 y preinclude al dejar de necesitar lectura.
MUST NOT modificar Drivers, servicios/otros dispositivos/contratos aceptados ni
inicializar/enlazar globalmente periféricos. Configuración MR0=1000, frecuencia,
potencia/adaptación/main y validación física de errata PWM.1 quedan al final.

#### Scenario: S5 — Verificación por modalidad
- Given wrapper directo, doble de escritura y módulos aceptados preservados.
- When se ejecutan controles/diff/hashes.
- Then MUST registrar evidencia actual, separada de contratos descartados y del hardware pendiente.

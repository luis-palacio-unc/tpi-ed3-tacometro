# Dispositivo Motor — candidato B15

Origen R03/R06/R10/R11/R15. Propuesta sin acuerdo, no contrato vigente.

## Requirements

### Requirement: MD1 — Mando normalizado y período explícito

Motor MUST exponer bool Motor_Write(uint16_t mando_por_mil, uint32_t periodo_ticks).
Mando válido MUST ser 0..1000 inclusivo; período válido 1..UINT32_MAX-1, en ticks
del PWM y coincidente con MR0 activo/estable. MUST preservar período/frecuencia,
convertir sin overflow y redondear al tick más cercano con empate hacia arriba.
MUST representar 0 % bajo continuo y 100 % alto continuo, sin interpretar RPM.

#### Scenario: S1 — Dominio y extremos
- Given período de 1000 ticks y mandos 0/500/1000.
- When se solicita cada mando.
- Then MUST enviar MR1=0/500/1001 respectivamente y devolver true, sin actualizar MR0.

#### Scenario: S2 — Resolución y límites aritméticos
- Given períodos 1/2/3/1001/UINT32_MAX-1 y mandos válidos intermedios/extremos.
- When se actualiza la salida, incluidas fracciones a medio tick.
- Then MUST mantener error máximo de medio tick y representar período completo con MR1=período+1.

### Requirement: MD2 — Rechazo y recuperación

MUST devolver false si mando >1000 o período es 0/UINT32_MAX, sin invocar salida
ni emitir cero automáticamente. Una solicitud válida posterior MUST operar sin
retener errores. true MUST significar despacho al driver, no diagnóstico físico.

#### Scenario: S3 — Entradas inválidas
- Given mandos 1001/UINT16_MAX o períodos inválidos, después de una orden válida.
- When se llama Motor_Write y luego se envía una orden válida.
- Then MUST rechazar sin llamadas y recuperar con un único despacho válido.

### Requirement: MD3 — Operación de dispositivo

MUST usar driver original para actualizar PWM1.1, propuesto P2.0 función 01.
MUST despachar una vez por solicitud válida, incluidas repeticiones, sin init,
reset/esperas/ISR/heap/contexto/estado propio. Uso físico MUST requerir recurso
exclusivo, configuración previa activa alta single-edge, reset por MR0, sin
otros matches que detengan/reinicien el contador y acceso serializado.
La actualización MUST respetar el latch del driver; no prometer efecto inmediato.

#### Scenario: S4 — Repetición y conservación
- Given PWM previamente configurado y una secuencia de mandos repetidos/cero/avance.
- When se emiten órdenes válidas.
- Then MUST actualizar sólo el match de salida una vez por orden, sin tocar MR0/otros dispositivos.

### Requirement: MD4 — Capas, documentación y verificación

MUST documentar ESW.2.1.9/USW.2.1.9.1 con Doxygen y verificar host/sanitizadores,
ARM, regresiones y biblioteca intacta. MUST NOT modificar Driver_Motor, otros
dispositivos/aplicación, inicializar/enlazar globalmente periféricos ni incorporar
regulación/dirección/freno. Configuración/frecuencia/cableado y adaptación del
servicio MUST quedar para el final; host MUST distinguirse de PWM físico.

#### Scenario: S5 — Evidencia por modalidad
- Given dispositivo, pruebas con doble de API y módulos aceptados preservados.
- When se ejecutan controles y se revisa el diff/hashes.
- Then MUST registrar sólo evidencia ejecutada, con PWM/motor reales pendientes.

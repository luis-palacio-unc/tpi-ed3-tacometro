# Dispositivo Motor — candidato B15 corregido

Origen R03/R06/R10/R11/R15. Simplificación autorizada por el usuario2026-11-10;
reemplaza el parámetro de período de c952892. Resultado pendiente de aceptación.

## Requirements

### Requirement: MD1 — Mando normalizado y período obtenido del dispositivo

Motor MUST exponer bool Motor_Write(uint16_t mando_por_mil), sin parámetro de
período. Mando válido MUST ser 0..1000. Para convertirlo MUST leer MR0 de PWM1
en cada llamada válida, sin caché ni configuración duplicada. MR0 válido MUST ser
1..UINT32_MAX-1 ticks. MUST preservar período/frecuencia, convertir sin overflow
y redondear al tick más cercano con empate arriba. MUST representar 0 % bajo
continuo y 100 % alto continuo, sin interpretar RPM.

#### Scenario: S1 — Dominio y extremos
- Given MR0=1000 y mandos0/500/1000.
- When se llama Motor_Write sólo con el mando.
- Then MUST enviar MR1=0/500/1001 y devolver true, sin escribir MR0.

#### Scenario: S2 — Resolución y límites aritméticos
- Given MR0=1/2/3/1001/UINT32_MAX-1 y mandos válidos intermedios/extremos.
- When se actualiza la salida, incluidas fracciones de medio tick.
- Then MUST mantener error máximo de medio tick; período completo usa MR1=MR0+1.

#### Scenario: S6 — Lectura actual de MR0
- Given mando250 y MR0=1000, luego reconfiguración externa terminada a2000.
- When se vuelve a llamar con el mismo mando.
- Then MUST enviar250 y500 respectivamente, sin conservar el período anterior.

### Requirement: MD2 — Rechazo y recuperación

MUST devolver false si mando>1000 o MR0 es0/UINT32_MAX, sin invocar escritura
ni emitir cero automáticamente. Una llamada válida posterior MUST recuperar
sin retener errores. true MUST significar despacho, no diagnóstico físico.

#### Scenario: S3 — Entradas o configuración inválidas
- Given mando1001/UINT16_MAX o MR0 inválido después de una orden válida.
- When se llama Motor_Write y después se restablecen valores válidos.
- Then MUST rechazar sin escritura y recuperar con un único despacho válido.

### Requirement: MD3 — Operación de dispositivo

MUST leer directamente LPC_PWM1->MR0 en Dispositivos, ya que la biblioteca no
ofrece getter, y usar PWM_MatchUpdateSingle original para PWM1.1/P2.0 función01.
MUST despachar una vez por solicitud válida, incluidas repeticiones; sin init,
reset/esperas/ISR/heap/contexto/estado propio. Uso físico MUST exigir acceso
al periférico habilitado, recurso exclusivo, single-edge activo alto y MR0
estable/efectivo sin actualización pendiente, reset por MR0 y sin otros matches
que alteren TC. Acceso serializado y latch al próximo ciclo; no efecto inmediato.

#### Scenario: S4 — Repetición y conservación
- Given PWM preconfigurado y órdenes repetidas/cero/avance.
- When se llama Motor_Write.
- Then MUST leer sin escribir MR0 y despachar sólo el match de salida una vez por orden válida.

### Requirement: MD4 — Capas, documentación y verificación

MUST actualizar Doxygen ESW.2.1.9/USW.2.1.9.1 y verificar host/san/ARM/regresiones.
MUST NOT modificar Drivers, servicios/otros dispositivos/aplicación, inicializar
o enlazar globalmente periféricos ni incorporar regulación/dirección/freno.
Pruebas host MUST modelar MR0 sin alterar biblioteca ni introducir hooks de
producción; configuración/frecuencia/potencia/adaptación/main quedan al final.

#### Scenario: S5 — Evidencia por modalidad
- Given dispositivo corregido y controles conservados.
- When se ejecutan pruebas/build/Doxygen y revisión diff/hashes.
- Then MUST registrar evidencia actual, separada de c952892 y del PWM/motor físico pendiente.

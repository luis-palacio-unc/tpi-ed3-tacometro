# Dispositivos GPIO — contrato vigente

Origen: R05/R07/R10/R11/R15 y aclaraciones del usuario. B12 corregido aceptado
el2026-11-10; implementación4260249 y corrección92b4d62. Integración física pendiente.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: DG1 — Contacto del selector
Selector MUST exponer bool Selector_Read(void), true para contacto activo/nivel1
de P2.6 y false para nivel0. MUST usar GPIO_ReadValue y considerar sólo el bit6.
MUST NOT escribir GPIO, resolver modo de aplicación, antirrebote ni estados temporales.

#### Scenario: S1 — Lectura y pines ajenos
- Given puerto con bit6 en0/1 y combinaciones de otros bits.
- When se consulta.
- Then MUST devolver false/true respectivamente, independiente de otros bits y sin escrituras.

### Requirement: DG2 — Polaridad propia de cada salida
LED/Buzzer MUST exponer void LED_Write(bool activar) y
void Buzzer_Write(bool activar), sin contexto ni valor de retorno. Pines fijos
privados: LED rojoP0.22, buzzerP2.7. Buzzer activo por alto: true MUST solicitar1
y false0. LED rojo activo por bajo: true MUST solicitar0 y false1. MUST usar
GPIO_SetPinState. Estas operaciones MUST NOT declarar diagnóstico eléctrico.
No agregar errores ficticios/caché/reintentos/estado propio ni lectura de confirmación.

#### Scenario: S2 — Polaridad y aislamiento
- Given salidas con estados de pines vecinos distintos.
- When se solicita activar y desactivar cada dispositivo.
- Then MUST solicitar buzzer1/0 y LED0/1 sólo en su pin, preservando vecinos y la otra salida.

#### Scenario: S3 — Repetición e independencia
- Given salidas accionadas y luego alteradas externamente o peticiones intercaladas.
- When se repiten o alternan órdenes directas a cada dispositivo.
- Then MUST aplicar cada orden sin caché y preservar la otra salida y el selector.

Durante B12 se conservó Driver_Alarmas/B07 para revisarlo por separado.
B07-R1 aceptado2026-11-10 ya permite composición directa void(bool), ver SA3/S9
de salidas-alarma; se verificó en host. La llamada desde aplicación/main queda
para integración final. MUST NOT introducir adaptadores ni casts de funciones.

### Requirement: DG3 — Capas y alcance
Dispositivos MUST interactuar con hardware mediante los drivers originales, sin
duplicar lógica de acceso. Biblioteca Drivers MUST conservarse byte a byte.
Selector_Read, LED_Write y Buzzer_Write MUST exigir su inicialización previa; uso
serializado. FIOMASK en su valor de reset es precondición. (Antes: MUST NOT
inicializar GPIO/PINSEL; configuración como precondición externa.)

#### Scenario: S4 — Evidencia limitada
- Given wrappers e inicializaciones compilados y GPIO/PINSEL emulados en host.
- When se verifica polaridad, bits, dirección y documentación.
- Then MUST distinguir esa evidencia del funcionamiento eléctrico en placa.

### Requirement: DG4 — Inicialización de pines
MUST exponer void Selector_Init(void), void LED_Init(void) y void Buzzer_Init(void),
con PINSEL_ConfigPin y GPIO_SetDir originales. Selector: P2.6 función 00, pull-up
interno, entrada (llave a GND: abierta lee 1). LED: P0.22 función 00, sin
resistencias, nivel 1 (apagado) cargado antes de pasar a salida. Buzzer: P2.7
función 00, sin resistencias, nivel 0 cargado antes de pasar a salida. Cada init
MUST tocar sólo su pin. La interpretación del selector MUST quedar fuera.

#### Scenario: S5 — Selector
- Given PINSEL y GPIO emulados.
- When se llama Selector_Init.
- Then MUST pedir pull-up y entrada en P2.6 sin escribir niveles.

#### Scenario: S6 — Salidas sin transitorio activo
- Given puertos con patrones distintos en los pines vecinos.
- When se llama LED_Init y Buzzer_Init.
- Then el nivel inactivo MUST estar cargado al cambiar la dirección, y los vecinos intactos.

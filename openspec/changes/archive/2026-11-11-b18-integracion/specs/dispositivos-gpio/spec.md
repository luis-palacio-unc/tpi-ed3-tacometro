# Delta B18/I1 — dispositivos-gpio

Candidato de la vía rápida B18. DG1 y DG2 sin cambios (lectura/escritura intactas).

## MODIFIED Requirements

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

## ADDED Requirements

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

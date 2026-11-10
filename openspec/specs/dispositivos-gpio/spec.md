# Dispositivos GPIO — contrato vigente

Origen: R05/R07/R10/R11/R15 y aclaraciones del usuario. B12 corregido aceptado
el2026-11-10; implementación4260249 y corrección92b4d62. Integración física pendiente.

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
Dispositivos MUST interactuar con hardware mediante los drivers originales,
reutilizando sus operaciones sin duplicar lógica de acceso o crear otra capa genérica.
Aplicación/servicios MUST conservar independencia del micro y contratos lógicos.
Biblioteca Drivers MUST conservarse byte a byte. MUST documentar USW y mantener
regresiones/build de módulos. MUST NOT inicializar GPIO/PINSEL/clock, enlazar
periféricos al arranque ni cambiar main/servicios/contratos aceptados en este bloque.
Configuración previa de GPIO/dirección/polarización y FIOMASK habilitando esos bits
MUST ser precondición externa; uso serializado. No garantizar estado antes de configurar.

#### Scenario: S4 — Evidencia limitada
- Given wrappers compilados y GPIO emulado en pruebas host.
- When se verifica polaridad, bits y documentación.
- Then MUST distinguir esa evidencia de funcionamiento eléctrico/inicialización en placa.

# Timer — delta candidato B12

Origen R01/R10/R15, B11 aceptado. Propuesta, sin autorización para implementar.

## MODIFIED Requirements

### Requirement: WT3 — Integración acotada
MUST conservar eliminación de Temporizacion.h y documentación/build verificables.
B12 MAY integrar arranque en main y soporte de reloj/enlace de plataforma, sin
modificar lógica de servicios aceptados ni API del wrapper. Drivers MUST permanecer
originales byte a byte. Nuevos contratos MUST permanecer candidatos hasta aceptación.

#### Scenario: S3 — Integración revisable
- Given B11 aceptado y biblioteca original.
- When se incorpora al firmware.
- Then MUST conservar regresiones, documentación y biblioteca, distinguiendo compilación de placa.

## ADDED Requirements

### Requirement: IT1 — Enlace y arranque
Firmware ARM MUST enlazar originales necesarios y resolver referencias TIM/CMSIS.
MUST ejecutar Timer_Init y Timer_Start una vez antes del lazo de aplicación.
Información SystemCoreClock MUST actualizarse antes de inicializar timer usando
función existente, sin incorporar registros/MCU en aplicación o servicios.

#### Scenario: B12-S1 — Ruta de arranque
- Given memoria inicializada y timer exclusivo desde reset.
- When arranca main.
- Then MUST haberse actualizado la información de reloj y ejecutar Init antes de Start.

### Requirement: IT2 — Evidencia y límites
Debug/Release y fixture que usa Timer_Read MUST enlazar sin símbolos indefinidos
y conservar B11/unidades. MUST verificar invariantes de imagen, regresiones, Doxygen
y preservación de Drivers. MUST NOT llamar SystemInit/reconfigurar PLL/flash ni
agregar esperas/ISR/servicios nuevos para mantener esta base.

#### Scenario: B12-S2 — Enlace completo
- Given fuentes reales y toolchain ARM.
- When se compilan imagen y fixture con las tres operaciones.
- Then MUST resolver driver/reloj/aserciones y verificar imagen, sin afirmar ejecución.

#### Scenario: B12-S3 — Ensayo físico pendiente
- Given programación/placa no autorizadas o no disponibles.
- When termina la verificación software.
- Then MUST registrar pendiente la comprobación de avance/base1ms, sin declararla ejecutada.

# Modos — contrato vigente

Origen: R05/R06 y R14 (aclaración del usuario: máximo 185 RPM); interfaces de R03/R04. Contratos anteriores en el historial archivado.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: MO1 — Tres estados
`Aplicacion/Procesamiento` MUST exponer
`Procesamiento_Estado Procesamiento_InicializarModo(Mandos_Modo)`,
`Procesamiento_Estado Procesamiento_ActualizarModo(Mandos_Modo, bool hay_consigna,
uint32_t consigna_rpm)` y `uint32_t Procesamiento_Consigna(void)`, con estados
MANUAL, AUTO_ESPERA y AUTO_ACTIVO en memoria estática. Inicializar: MANUAL →
MANUAL; AUTOMATICO → AUTO_ESPERA; consigna 0 e integral del regulador en cero.

#### Scenario: S1 — Arranque (`i2.procesamiento`)
- Given modo inicial MANUAL o AUTOMATICO.
- When se inicializa y se actualiza en automático sin consigna.
- Then MUST dar MANUAL o AUTO_ESPERA, consigna 0, y seguir en AUTO_ESPERA.

### Requirement: MO2 — Transiciones
Actualizar MUST aplicar, en orden: (1) modo MANUAL → MANUAL, consigna 0 e
integral del regulador en cero (regulacion-velocidad RG3);
(2) de MANUAL a modo AUTOMATICO → AUTO_ESPERA, descartando la consigna de esa
llamada; (3) en AUTO_ESPERA/AUTO_ACTIVO, `hay_consigna` con valor 0..185 →
AUTO_ACTIVO con esa consigna; un valor mayor MUST ignorarse sin alterar estado ni
consigna. Sin consigna nueva MUST conservar estado y consigna.

#### Scenario: S2 — Consigna activa (`i2.procesamiento`)
- Given AUTO_ESPERA.
- When llegan 100, nada, 0 y 185.
- Then AUTO_ACTIVO con 100, 100, 0 y 185; un valor con `hay_consigna` false MUST ignorarse.

#### Scenario: S3 — Fuera de rango (`i2.procesamiento`)
- Given AUTO_ESPERA, y AUTO_ACTIVO con 184.
- When llegan 186 o UINT32_MAX.
- Then MUST seguir en AUTO_ESPERA con 0, o en AUTO_ACTIVO con 184.

#### Scenario: S4 — Entrada en automático (`i2.procesamiento`)
- Given MANUAL (consignas ignoradas, consigna 0).
- When el modo pasa a AUTOMATICO con consigna 100 en la misma llamada; luego 120;
  luego MANUAL y otra vez AUTOMATICO sin consigna.
- Then AUTO_ESPERA con 0; AUTO_ACTIVO con 120; MANUAL con 0; AUTO_ESPERA con 0,
  sin reutilizar 120.

# Salidas de alarma — contrato vigente

Origen R07/R10/R11/R15. Contratos anteriores en el historial archivado.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: SA5 — LED y buzzer juntos
`Servicios/Driver_Alarmas` MUST exponer `void Driver_Alarmas_Init(void)` (llama
`LED_Init` y `Buzzer_Init`, ambos apagados) y `void Driver_Alarmas_Aplicar(bool
activa)`, que entrega el mismo valor a `LED_Write` y a `Buzzer_Write`. La polaridad
eléctrica es de cada dispositivo.

#### Scenario: S1 — LED y buzzer juntos (`i2.drivers`)
- Given dispositivos simulados.
- When se inicializa y se aplica true, true, false, true, false, false.
- Then Init MUST NOT escribir salidas; cada aplicación MUST escribir una vez cada
  salida con el valor pedido.

#### Scenario: S2 — Polaridad real (`b12.dispositivos_gpio` S9)
- Given `LED.c` y `Buzzer.c` reales sobre GPIO emulado.
- When se aplica el servicio.
- Then activa MUST dar P0.22 bajo y P2.7 alto; inactiva lo inverso; sin tocar otros pines.

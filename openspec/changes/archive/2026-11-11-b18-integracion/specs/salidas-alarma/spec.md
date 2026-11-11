# Delta B18/I2 — salidas-alarma (reemplaza el contrato vigente)

Candidato de la vía rápida B18. SA1–SA4 (B07/B07-R1) quedan como historial.

## REMOVED Requirements

### Requirement: SA1–SA4
Motivo: estructura con dos punteros a función y enum de resultado. El servicio
pasa a llamar a los dispositivos LED y Buzzer.

## ADDED Requirements

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

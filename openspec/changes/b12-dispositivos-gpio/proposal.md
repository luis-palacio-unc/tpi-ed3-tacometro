# B12 — Selector, LED y Buzzer mediante GPIO

## Objetivo y origen

Completar tres elementos de Dispositivos del anteproyecto (R05/R07/R10/R11/R15).
El usuario pospone inicialización/enlace y autoriza selector/buzzer activos por alto
en pines P2 distintos de EINTx. Alcance autorizado el2026-11-10; resultado pendiente de aceptación.

## Alcance

- Selector_Read: leer el contacto en P2.6 mediante GPIO_ReadValue; true si nivel1.
  Entregar sólo actividad lógica; antirrebote y selección de modo siguen en Mandos.
- LED_Write y Buzzer_Write: mando lógico mediante GPIO_SetPinState. BuzzerP2.7:
  activo→1, inactivo→0. LED rojoP0.22: activo→0, inactivo→1 según conexión de placa.
- Firmas de salida compatibles con Driver_Alarmas existente, sin cambiar servicios.
  Contexto opaco ignorado; aceptación de llamada no acredita estado eléctrico.
- Dispositivos interactúa con hardware y drivers directamente, con nombres cortos y Doxygen.
- Comprobar traducción lógica, selección de bit y preservación de pines ajenos.

Archivos afectados: Dispositivos/Selector.*, LED.*, Buzzer.*, pruebas host, CMake,
Doxyfile y seguimiento SDD. No modificar ningún archivo de Drivers.
Excluir configurar periféricos/PINSEL/dirección/polarización, main/startup/reloj, enlace
de biblioteca, antirrebote nuevo, control de alarmas, PWM/ADC/UART/captura óptica,
flasheo y pruebas físicas. Esas operaciones requieren configuración previa al usar
los wrappers en placa, que se hará en la integración final.

## Dependencias y criterio de aceptación

B07/B10 y driver GPIO originales. Manual UM10360 rev4.1 tabla84/p120 confirma
que P2.6/P2.7 no son EINT0–3; esquema OM13085 hoja2 confirma LED rojoP0.22 bajo.
Revisión física de la placa y estado de máscaras/dirección se comprobarán al integrar.

1. Selector distingue0/1 del bit asignado, ignorando los demás; no escribe GPIO.
2. Activar/desactivar buzzer produce1/0, LED0/1, sólo en su pin y compone con B07.
3. Servicios conservan contrato lógico; Dispositivos usa las operaciones del driver,
   sin duplicar máscaras/escrituras SET/CLR ya resueltas por la biblioteca.
4. Pruebas host de polaridad/aislamiento, compilación ARM de módulos, regresiones y
   Doxygen pasan; hashes Drivers originales intactos. No declarar ensayos físicos.

Reversión: commit inverso de B12, preservando biblioteca y B11. Orden posterior
tentativo: ADC-DMA/entradas analógicas, sensor óptico, motor y UART, por bloques acordados.

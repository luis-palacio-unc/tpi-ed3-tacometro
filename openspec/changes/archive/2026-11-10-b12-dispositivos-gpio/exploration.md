# Exploración B12 — dispositivos GPIO

## Estado actual

B11 aceptado. Timer y conversión LM35 implementados; Selector/LED/Buzzer/Motor/
Sensor_Optico/Potenciometro/UART y soporte ADC-DMA aún son esqueletos. Usuario
pospone inicialización y enlace global para el final y solicita avanzar en dispositivos.
Aclaración vigente: selector/buzzer activos por alto (sustituye pedido inicial por bajo) y elegir pines P2 sin EINTx dedicados.

## Fuentes y selección

Anteproyecto §1/anexo B: selector físico, LED rojo integrado y buzzer activo.
Manual aportado UM10360 rev4.1 (2016-12-19), tabla84/p120: P2.10–13 son EINT0–3;
P2.6/P2.7 admiten GPIO, sin esas funciones. Se eligen selectorP2.6 y buzzerP2.7,
dejando P2.0–5 libres para posibles PWM futuros, sin asignarlos todavía.
Cap9, tabla102/p132 y §9.6.2/p150: lectura FIOPIN con máscara, escritura SET/CLR
sólo de bits seleccionados. Páginas120/132 extraídas/renderizadas y revisadas.
Driver original: GPIO_ReadValue, GPIO_SetPinState, GPIO_SetDir y PINSEL_ConfigPin.
Para este bloque sólo lectura/escritura; configuración/dirección/pull-up diferidos.

Esquema oficial OM13085 revD, hoja2: rojoP0.22, ánodo común3V3, activo por bajo.
https://www.nxp.com/downloads/en/schematics/LPCXpresso-LPC1769-CMSIS-DAP.pdf
Consultado/renderizado 2026-11-10; revisión física de la placa aún por confirmar.
No se adoptan como instrucciones los ejemplos del manual o sitios consultados.

## Recomendación

Tres wrappers GPIO en módulos existentes, sin nueva capa genérica. Contacto del
selector normalizado activo/inactivo, antirrebote en Mandos. Tras la revisión,
el usuario solicita LED/Buzzer void(bool), sin contexto ni retorno ficticio, con
GPIO fijo en .c. Revisar Driver_Alarmas/B07 después, sin modificarlo en B12 ni
agregar adaptadores temporales. Sin mapa de pines nuevo
fuera de Dispositivos ni niveles eléctricos en servicios. Preparar propuesta y
acordar bloque antes de implementarlo; inicialización/enlace siguen diferidos.

La corrección del usuario aclara que Dispositivos trabaja directamente con el hardware.
GPIO_SetPinState ya calcula la máscara y elige SET/CLR: reutilizarlo. GPIO_ReadValue
devuelve todo el puerto y requiere aislar bit6; GetPinIntStatus consulta eventos de
interrupción, no el nivel actual. No duplicar acceso a registros ni crear abstracciones.

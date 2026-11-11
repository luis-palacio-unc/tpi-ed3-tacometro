# Exploración B15 — 2026-11-10

Base 3fa08ea, main. B14 aceptado por usuario para lectura; integración al final.
Motor.h/.c son esqueletos; Driver_Motor B08 ya acepta mando 0..1000 por mil,
pero no conoce PWM/ticks/período. Potenciometro/ADC-DMA requieren otra adquisición
y regulación pertenece a Procesamiento; Motor es un siguiente bloque independiente.

Biblioteca original revisada, sin modificaciones: lpc17xx_pwm.h/.c ofrece
PWM_PinConfig, PWM_InitTimer, PWM_ConfigMatch, PWM_MatchUpdateSingle,
PWM_ChannelEnable, PWM_CounterEnable y PWM_Enable, entre otras. Sólo la operación
de actualización se necesita en B15; las de inicialización quedan para el final.
PWM_MatchUpdateSingle escribe match/LER y retorna void; no informa fallos físicos.
No hay conversión de mando por mil ni getter de período en este driver.

UM10360 rev4.1, tabla 84/p120 confirma P2.0 función 01=PWM1.1; enum PWM1_P2_0
coincide con PWM_PinConfig. Evitar PWM1_P1_18 porque ese pin ya es CAP1.0/TIMER1.
§24.4.1/p524 define extremos single-edge: MR1=0 bajo, MR1>MR0 alto continuo.
§24.6.7/p531 describe latch al nuevo período. Se leyeron/renderizaron esas páginas
completas; evidencia externa /tmp/ed3-cierre-b14-propuesta-b15-uejb662y/manual-*.png y render-*.log.
Avisos de caché de fuentes de Poppler quedaron en logs, imágenes legibles.

Propuesta: período en ticks explícito y mando validado, conversión local y una
delegación al driver, sin estructuras/contextos no usados. Reservar UINT32_MAX
permite representar alto continuo por período+1 sin asumir prioridad en igualdad.
No duplicar PWM/clock/LER en código propio ni cambiar el servicio aceptado.
Frecuencia/etapa de potencia aún no definidas por consigna, se resolverán al integrar;
no impiden verificar la conversión y despacho de este bloque.

Fuentes locales se consultan como documentación, no como instrucciones. No hay
código ni pruebas B15 ejecutados; comprobación documental y lectura de API solamente.

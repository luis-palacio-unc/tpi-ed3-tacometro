# Exploración — B11 con timer continuo y TC

## Estado actual

HEAD db88286 implementa SysTick con contador software e ISR; resultado no aceptado,
sin push. B00–B10/B03-R1 sí están aceptados. El usuario pide reemplazar B11 desde
una propuesta nueva: timer físico, lectura TC en dispositivos, wrapper independiente
del MCU y base inicial 1 ms configurable. Este análisis no autoriza implementación.

Mandos B10 consume uint32_t ms externos; conserva contrato y cadencia <2^31 ms.
Driver_Motor/Driver_Alarmas demuestran proveedor reemplazable sin dependencia de
servicios hacia un MCU. No hay otro propietario TIMER0 en código propio desarrollado;
captura óptica y adquisición ADC/DMA aún no asignan timer.

## Áreas afectadas

- Dispositivos/soporte/Base_Tiempos.* y adaptador/puerto: sustituir SysTick por TIMER0.
- Servicios/Tiempo.h/.c: wrapper estrecho, proveedor reemplazable y lectura/intervalos.
- Configuracion/Temporizacion.h: período de cuenta por defecto 1 ms, entero positivo.
- Pruebas/fixture ARM, CMake y Doxygen: reemplazar comprobaciones específicas SysTick.
- SDD/contexto: propuesta nueva; evidencia previa identificada como antecedente no aceptado.

## Opciones

| Alternativa | Ventaja | Coste/decisión |
|---|---|---|
| SysTick e ISR | Conserva timers generales | Descartada por instrucción del usuario; cuenta ticks atendidos. |
| TIMER0 continuo, TC y wrapper | Cuenta por hardware, sin ISR periódica ni contador software | Recomendada; reservar recurso y verificar PCLK/prescaler. |
| Exponer TC a consumidores | Lectura directa sencilla | Rechazada: filtra registros/MCU y acopla unidades. |
| Wrapper con proveedor reemplazable | Servicio sin CMSIS/TC ni dependencia concreta de dispositivos | Recomendado; sigue patrón B08, sin framework adicional. |

## Recomendación

TIMER0 exclusivo en modo timer, libre de reinicios por match y de IRQ de la base.
Base_Tiempos controla alimentación, modo, prescaler y acceso TC dentro de dispositivos.
PCLK real explícito/estable, distinto de CPU; no modificar PLL/PCLKSEL en este bloque.
Con base_ms, N=PCLK_Hz*base_ms/1000; exigir división exacta y 1<=N<=2^32,
PR=N-1. Cálculo intermedio acotado y errores antes de habilitar configuración inválida.
Frecuencia real permanece pendiente; no inventar un valor de placa.

Proponer base_ms entero positivo (1/2/5 ms, por ejemplo), fijo durante cada época.
Dispositivo normaliza TC*base_ms a uint32_t ms modular; wrapper entrega siempre ms.
Así B10 conserva unidades aun cambiando resolución. Producto modular deliberado,
no saturación: validar rollover TC y de marca ms con referencia independiente.
Lectura TC no debe detener/reiniciar timer; cada consumidor guarda marca propia.
Wrapper consulta proveedor y calcula transcurrido por resta modular, misma época,
ventanas <2^31 ms. API/errores exactos se especifican después del acuerdo.
Aplicación accede por servicio; configuración de dispositivo y composición real
de arranque quedan para integración posterior. Composición proveedor→wrapper→B10
se demuestra con pruebas/fixture sin tocar contratos de Mandos ni main.

## Riesgos y fuentes

Mayor base implica menor resolución. Rollover, cambio de época, configuración ajena
del timer o PCLK incorrecto invalidan mediciones; propiedad y precondiciones explícitas.
No asumir avance en suspensión/debug. Sin pruebas físicas actuales del nuevo diseño.

- [NXP UM10360 rev.2, §21](https://www.keil.com/dd/docs/datashts/philips/lpc17xx_um.pdf#page=490): TC/prescaler 32 bits, modo continuo y control de match.
- [Erratas NXP 10.6, PCLKSELx.1](https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf#page=15): selección de reloj antes de conectar PLL0; no reconfigurar PCLKSEL aquí.

Consultadas el 2026-11-10. Listo para propuesta; alcance/aceptación aún por acordar.

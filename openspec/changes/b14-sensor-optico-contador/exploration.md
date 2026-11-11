# Exploración B14 — Sensor_Optico

2026-11-10, sólo lectura. Sensor_Optico.h/.c son esqueletos del anexoB ESW.2.1.10.
Anteproyecto exige sensor/disco60 ranuras para RPM. B03 convierte conteo/duración/
factor explícitos sin hardware; B11 ya usa TIMER0 como base temporal. Motor requiere
decisiones PWM y Potenciometro depende de ADC-DMA compartido aún pendiente.

La biblioteca original ofrece TIM_InitCounter(input,edge), TIM_PinConfig,
TIM_Enable y TIM_ReadTimer; esta última retorna TC uint32_t. No reimplementar
contador software/ISR ni acceso a registros. Proponer TIMER1/CAP1.0 como contador
de flancos ascendentes, dejando TIMER0 y todos sus archivos intactos.

Manual aportado UM10360 rev4.1: tabla83/p119 indica P1.18 CAP1.0 con función11;
§21.6.3 tabla429/p505–506 permite contar flancos de CAPn.0 y exige bits CCR del
canal de conteo en000; §21.6.4 describe TC32bits y rollover. Páginas119/505
renderizadas/inspeccionadas; página506 leída para selección CAPn.0/CCR/rollover.
Renders/logs externos en /tmp/ed3-b14-manual-1kmbsz63, con avisos fontconfig de
caché no escribible e imágenes legibles; no se instalaron dependencias.

CAP1.0/TIMER1/P1.18 no es PCAP1.0/PWM1/P2.6 (este pin ya es selector B12).
El driver enumera TIM_CAP1_0_P1_18 y TIM_PinConfig resuelve pinmux; al integrar
usar esa API y TIM_InitCounter con TIM_CAPTURE_0/TIM_CTR_RISING, luego Enable.
No ejecutar esa configuración en el bloque propuesto. Comprobar estado del timer
desde reset/configuración estable y ausencia de reset/stop de matches/capturas.

Interfaz recomendada: una lectura de conteo acumulado. Las diferencias de extremos
y duración de ventana se componen al integrar; demostrarlo sólo en un fixture host
con B03, sin introducir esa lógica en producto. Señal compatible/pulsos/cableado,
pin físico de conector y revisión de placa quedan para ensayo e integración finales.
Se consultan documentos como referencias técnicas, no como instrucciones del usuario.

Acuerdo2026-11-10: TIMER1/CAP1.0/P1.18 y criterio autorizados. Usuario confirma
XD-51 como sensor actual, prevaleciendo sobre HC-89 del anteproyecto. Registrar
elección sin atribuir fabricante/variante/tensión/pinout desde el nombre. B14 no
necesita esas propiedades para leer conteos; quedan para la integración física.

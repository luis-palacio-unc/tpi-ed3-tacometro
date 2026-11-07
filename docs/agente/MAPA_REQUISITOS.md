# Mapa de requisitos

AP = anteproyecto.pdf §1; APA/APB = anexos A/B. G = guía REV 2.0.
U = instrucciones del usuario de incorporación SDD (2026-11-06).
Los IDs siguientes son locales de trazabilidad, no numeración de la consigna.
Las rutas indican módulos previstos; no acreditan implementación.

| ID / fuente | Requisito | Bloque | Archivos principales | Evidencia de aceptación |
|---|---|---|---|---|
| R01 AP | Ejecutar firmware en LPC1769/OM13085 | B00 aceptado: base de build | CMakeLists.txt, Aplicacion/main.c, cmake/, plataforma/ | S1/S3/S5/S6 PASS de build/inspección (verify-report B00); ejecución física pendiente |
| R02 AP/APA | Medir RPM con disco de 60 ranuras | Por planificar | Servicios/Medicion_Velocidad.*, Dispositivos/Sensor_Optico.* | Casos de pulsos/tiempo y medición con señal real; pendiente |
| R03 AP | Potenciómetro determina PWM en manual | Por planificar | Servicios/Mandos.*, Aplicacion/Procesamiento.*, Servicios/Driver_Motor.* | Límites y recorrido en host; PWM físico; pendiente |
| R04 AP | Regular RPM con consigna UART en automático | Por planificar | Aplicacion/Procesamiento.*, Servicios/Comunicacion.* | Ensayos de regulación y perturbación sobre motor; pendiente |
| R05 AP | Exclusión de fuentes de mando por modo | B01 verificado, pendiente de aceptación | Aplicacion/Procesamiento.* | B01 S1/S2 PASS (C host); integración física pendiente |
| R06 AP | Entrar en automático con PWM cero hasta nueva consigna válida | B01 verificado, pendiente de aceptación | Aplicacion/Procesamiento.*, Servicios/Comunicacion.* | B01 S3/S4/S5/S6 PASS (C host); recepción real pendiente |
| R07 AP | Temperatura supera umbral: LED rojo y buzzer prioritarios | Por planificar | Servicios/Temperatura.*, Driver_Alarmas.*, Aplicacion/Procesamiento.* | Frontera de umbral y latencia de alarma medidos; pendiente |
| R08 AP | Adquirir potenciómetro/temperatura con ADC y DMA | Por planificar | Dispositivos/LM35.*, Potenciometro.*, soporte/Adquisicion_ADC_DMA.* | Secuencia/canales, transferencia y coherencia sobre placa; pendiente |
| R09 AP | UART recibe consignas y transmite RPM, temperatura y estado | Por planificar | Servicios/Comunicacion.*, Dispositivos/UART.* | Mensajes válidos/rotos, recuperación y enlace físico; pendiente |
| R10 AP/APB | Tres capas; Procesamiento integra control; acceso mediante servicios | Transversal | Aplicacion/, Servicios/, Dispositivos/ | Revisión de dependencias por bloque; pendiente |
| R11 G §4 | Documentar ESW/USW y diagramas | Transversal | .c/.h propios y futura documentación de entrega | B01: 6 USW Procesamiento + main, Doxygen estricto PASS; resto pendiente |
| R12 G entrega | Firmware completo e informe en estructura de entrega | Bloque final por planificar | Futuros Firmware/ e Informe/ de distribución | Revisión de paquete; pendiente |
| R13 U | Persistencia SDD y verificaciones reproducibles por bloque | Incorporación / B00 | docs/agente/, openspec/, scripts/ | B00 aceptado/archivado; contrato build vigente; evidencia en archivo B00 |

R14 — aclaración del usuario (2026-11-06): aceptar consignas hasta **185 RPM**,
inclusive. B01: Procesamiento.c/.h; prueba LimitesRPM cubre 0/184/185 aceptadas y
186/UINT32_MAX rechazadas en espera y con consigna activa. Evidencia: corrección R14
en verify-report.md. Se conserva consigna activa ante rechazo; no se satura.

Aviso de temperatura en PC: opcional según AP; no ampliar alcance obligatorio.
B00 aceptado y archivado para su alcance de build/inspección. R01 conserva aceptación
física pendiente. B01 verificado en host y enlace ARM, sin aceptación todavía; no hay ensayos físicos.

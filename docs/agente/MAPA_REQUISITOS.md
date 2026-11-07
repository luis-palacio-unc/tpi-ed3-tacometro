# Mapa de requisitos

AP = anteproyecto.pdf §1; APA/APB = anexos A/B. G = guía REV 2.0.
U = instrucciones del usuario de incorporación SDD (2026-11-06).
Los IDs siguientes son locales de trazabilidad, no numeración de la consigna.
Las rutas indican módulos previstos; no acreditan implementación.

| ID / fuente | Requisito | Bloque | Archivos principales | Evidencia de aceptación |
|---|---|---|---|---|
| R01 AP | Ejecutar firmware en LPC1769/OM13085 | B00 aceptado: base de build | CMakeLists.txt, Aplicacion/main.c, cmake/, plataforma/ | S1/S3/S5/S6 PASS de build/inspección (verify-report B00); ejecución física pendiente |
| R02 AP/APA | Medir RPM con disco de 60 ranuras | B03 aceptado/archivado para cálculo; captura física pendiente | Servicios/Medicion_Velocidad.*, Dispositivos/Sensor_Optico.* | B03 S1–S7 PASS host/enlace ARM (verify-report archivado); medición con señal real pendiente |
| R03 AP | Potenciómetro determina PWM en manual | B04 aceptado/archivado para normalización; ADC/PWM físicos pendientes | Servicios/Mandos.*, Aplicacion/Procesamiento.*, Servicios/Driver_Motor.* | B04 S1–S7 PASS host/enlace ARM (verify-report archivado); ADC/PWM físicos pendientes |
| R04 AP | Regular RPM con consigna UART en automático | Por planificar | Aplicacion/Procesamiento.*, Servicios/Comunicacion.* | Ensayos de regulación y perturbación sobre motor; pendiente |
| R05 AP | Exclusión de fuentes de mando por modo | B01 aceptado; lógica | Aplicacion/Procesamiento.* | B01 S1/S2 PASS (C host); integración física pendiente |
| R06 AP | Entrar en automático con PWM cero hasta nueva consigna válida | B01 aceptado; lógica | Aplicacion/Procesamiento.*, Servicios/Comunicacion.* | B01 S3/S4/S5/S6 PASS (C host); recepción real pendiente |
| R07 AP | Temperatura supera umbral: LED rojo y buzzer prioritarios | Por planificar | Servicios/Temperatura.*, Driver_Alarmas.*, Aplicacion/Procesamiento.* | Frontera de umbral y latencia de alarma medidos; pendiente |
| R08 AP | Adquirir potenciómetro/temperatura con ADC y DMA | Por planificar | Dispositivos/LM35.*, Potenciometro.*, soporte/Adquisicion_ADC_DMA.* | Secuencia/canales, transferencia y coherencia sobre placa; pendiente |
| R09 AP | UART recibe consignas y transmite RPM, temperatura y estado | B02 aceptado para recepción lógica; resto pendiente | Servicios/Comunicacion.*, Dispositivos/UART.* | B02 S1–S8 PASS host; UART física/telemetría pendientes |
| R10 AP/APB | Tres capas; Procesamiento integra control; acceso mediante servicios | Transversal | Aplicacion/, Servicios/, Dispositivos/ | Revisión de dependencias por bloque; pendiente |
| R15 U, aclaración de diseño | Aplicación portable entre configuraciones físicas; separación de abstracciones exigida por el profesor según usuario | Transversal | Aplicacion/, Servicios/, Dispositivos/ | B04 escalas equivalentes y adaptación B01 PASS; dependencias revisadas; adquisición física pendiente |
| R11 G §4 | Documentar ESW/USW y diagramas | Transversal | .c/.h propios y futura documentación de entrega | B01–B04: 17 funciones/USW incluyendo main; Doxygen estricto PASS; resto pendiente |
| R12 G entrega | Firmware completo e informe en estructura de entrega | Bloque final por planificar | Futuros Firmware/ e Informe/ de distribución | Revisión de paquete; pendiente |
| R13 U | Persistencia SDD y verificaciones reproducibles por bloque | Incorporación / B00 | docs/agente/, openspec/, scripts/ | B00 aceptado/archivado; contrato build vigente; evidencia en archivo B00 |

R14 — aclaración del usuario (2026-11-06): aceptar consignas hasta **185 RPM**,
inclusive. Fuente común Configuracion/Limites.h (B02); alias públicos de servicios/aplicación.
B01: Procesamiento.c/.h; prueba LimitesRPM cubre 0/184/185 aceptadas y
186/UINT32_MAX rechazadas en espera y con consigna activa. Evidencia: corrección R14
en verify-report.md. Se conserva consigna activa ante rechazo; no se satura.

Aviso de temperatura en PC: opcional según AP; no ampliar alcance obligatorio.
B00 aceptado y archivado para su alcance de build/inspección. R01 conserva aceptación
física pendiente. B01 aceptado con R14 para lógica en host/enlace ARM; no hay ensayos físicos.
B02 aceptado para recepción lógica en host/enlace ARM; UART física pendiente.
B03 aceptado/archivado para cálculo y validez; S1–S7 PASS y 15 casos C.
Sin evidencia de medición física; contrato vigente en openspec/specs/medicion-velocidad/.

B04 aceptado/archivado: 15 casos C, recorridos/fronteras, sanitizadores y enlace ARM PASS.
Contrato vigente de mandos; adquisición del potenciómetro y PWM físicos pendientes.

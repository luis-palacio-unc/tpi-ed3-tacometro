# Mapa de requisitos

AP = anteproyecto.pdf §1; APA/APB = anexos A/B. G = guía REV 2.0.
U = instrucciones del usuario de incorporación SDD (2026-11-06).
Los IDs siguientes son locales de trazabilidad, no numeración de la consigna.
Las rutas indican módulos previstos; no acreditan implementación.

| ID / fuente | Requisito | Bloque | Archivos principales | Evidencia de aceptación |
|---|---|---|---|---|
| R01 AP | Ejecutar firmware en LPC1769/OM13085 | B00 aceptado: base de build | CMakeLists.txt, Aplicacion/main.c, cmake/, plataforma/ | S1/S3/S5/S6 PASS de build/inspección (verify-report B00); ejecución física pendiente |
| R02 AP/APA | Medir RPM con disco de 60 ranuras | B03 aceptado/archivado; revisión B03-R1 aceptada/archivada; captura pendiente | Servicios/Medicion_Velocidad.*, Dispositivos/Sensor_Optico.* | B03-R1 S1–S10 PASS, 23 casos + oráculo 11199; revisión aceptada; señal real pendiente |
| R03 AP | Potenciómetro determina PWM en manual | B04 aceptado para normalización; B08 aceptado/archivado para mando portable; ADC/PWM físicos pendientes | Servicios/Mandos.*, Aplicacion/Procesamiento.*, Servicios/Driver_Motor.* | B04 S1–S7 PASS host/enlace ARM (verify-report archivado); ADC/PWM físicos pendientes |
| R04 AP | Regular RPM con consigna UART en automático | Por planificar | Aplicacion/Procesamiento.*, Servicios/Comunicacion.* | Ensayos de regulación y perturbación sobre motor; pendiente |
| R05 AP | Exclusión de fuentes de mando por modo | B01 aceptado; B10 aceptado/archivado para selector estable | Aplicacion/Procesamiento.*, Servicios/Mandos.* | B01 S1/S2 y B10 SE1–SE4 PASS lógico; integración física pendiente |
| R06 AP | Entrar en automático con PWM cero hasta nueva consigna válida | B01/B08/B10 aceptados/archivados para transición y solicitud cero | Aplicacion/Procesamiento.*, Servicios/Comunicacion.*, Mandos.*, Driver_Motor.* | B01 S3–S6 y B10-S6 PASS lógico; recepción/actuación reales pendientes |
| R07 AP | Temperatura supera umbral: LED rojo y buzzer prioritarios | B05 corregido, aceptado y archivado: servicio genérico + LM35; B06 aceptado/archivado para decisión; B07 aceptado/archivado para coordinación; actuación física/adquisición pendientes | Servicios/Temperatura.*, Dispositivos/LM35.*, Driver_Alarmas.*, Aplicacion/Procesamiento.* | B05 S1–S8 PASS host/enlace ARM (verify-report archivado); B06 S1–S8 y 13 casos PASS; resultado lógico aceptado; alarma física pendiente |
| R08 AP | Adquirir potenciómetro/temperatura con ADC y DMA | Por planificar | Dispositivos/LM35.*, Potenciometro.*, soporte/Adquisicion_ADC_DMA.* | Secuencia/canales, transferencia y coherencia sobre placa; pendiente |
| R09 AP | UART recibe consignas y transmite RPM, temperatura y estado | B02 aceptado para recepción; B09 aceptado/archivado para codificación lógica TX; UART/envío pendientes | Servicios/Comunicacion.*, Dispositivos/UART.* | B02 RX y B09 CT1–CT4 PASS lógico; UART/envío físicos pendientes |
| R10 AP/APB | Tres capas; Procesamiento integra control; acceso mediante servicios | Transversal | Aplicacion/, Servicios/, Dispositivos/ | Revisión de dependencias por bloque; pendiente |
| R15 U, aclaración de diseño | Aplicación y servicios independientes del hardware específico; adaptación del sensor en dispositivos (aclaración del usuario) | Transversal | Aplicacion/, Servicios/, Dispositivos/ | B05 servicio aislado/fuentes genéricas PASS; curva en LM35. B03-R1 recibe factor explícito sin configuración/dispositivos, aceptado/archivado |
| R11 G §4 | Documentar ESW/USW y diagramas | Transversal | .c/.h propios y futura documentación de entrega | B01–B10: 32 funciones/USW, incluidas tres nuevas USW.2.1.5.3–.5; Doxygen estricto PASS; resto pendiente |
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

B05 corregido, aceptado y archivado: 10 casos del servicio genérico, 17 LM35 y 3 integración;
regresiones/sanitizadores/ARM/Doxygen PASS. Aclaración R15 incluye independencia
de servicios, con curva específica en dispositivos. B03-R1 aceptado por separado el 2026-11-09.
Contrato vigente de temperatura sincronizado; adquisición y alarma aún pendientes.

B03-R1: factor TPI en Configuracion/Medicion.h, servicio genérico con factor explícito.
23 casos C + 11199 ternas de oráculo independiente y sanitizadores PASS;
ARM Debug/Release y Doxygen PASS. Revisión aceptada/archivada; contrato vigente
sincronizado y archivo B03 original intacto. Captura/tiempos físicos siguen pendientes.

B06 aceptado/archivado (R07/R10/R11/R15): decisión
térmica en Procesamiento con lectura B05 y umbral explícito, sin modelo de sensor.
AT1–AT4/S1–S8 PASS: 13 casos B06, 115 C totales + 11199 ternas de regresión;
san/ARM/Doxygen PASS. Informe archivado B06 y contrato vigente de alarma-temperatura. Señalización física/prioridad temporal
y respuesta física ante fallo de lectura quedan para integración posterior.

B07 aceptado/archivado (R07/R10/R11/R15): servicio
Driver_Alarmas portable con interfaz reemplazable y fallos observables, sin dependencia
de aplicación/dispositivos. SA1–SA4/S1–S8 PASS: 12 casos de servicio + cuatro integración,
131 C totales y 11199 ternas de regresión; san/ARM/Doxygen PASS. Evidencia archivada B07 y contrato vigente salidas-alarma.
Proveedores emulados no acreditan GPIO, polaridad, señalización real o prioridad temporal.

B08 aceptado/archivado (R03/R06/R10/R11/R15):
Driver_Motor valida/envía mando por mil y cero mediante salida reemplazable.
MM1–MM4/S1–S8 PASS: 10 casos servicio + 5 integración B04/B01; 146 C y oráculo
11199, host/san 13/13, ARM Debug 11/11 y fixture B08 Release; Doxygen 25 funciones.
Matriz/evidencia en [informe B08](../../openspec/changes/archive/2026-11-09-b08-mando-motor-portable/verify-report.md); PWM/potencia/control/main aún pendientes.
R11: planificación de diagramas acordada en [CONTEXTO](CONTEXTO.md#documentación-final-y-b08); elaboración pendiente.

B09 aceptado/archivado (R09/R10/R11/R15):
codificación ASCII RPM/temperatura/estado con ND/ERR y buffer acotado. CT1–CT4/B09-S1–S8
y delta B02-R5/S9 PASS: 11 casos codec + cuatro integración B01/B03/B05/RX, 161 C,
5096 pares codec y 11199 ternas B03-R1; host/san 15/15, ARM Debug 12/12 y fixture
B09 Release; Doxygen 29 funciones/USW. [Matriz/evidencia](../../openspec/changes/archive/2026-11-09-b09-codificacion-telemetria/verify-report.md).
UART/envío/periodicidad/main pendientes; no se acredita recepción física PC.

B10 aceptado/archivado (R05/R06/R10/R11/R15):
selector lógico estable en Mandos, intervalo configurable y reloj externo. MA3/S6 y
SE1–SE4/B10-S1–S6 PASS: 14 casos selector + cuatro integración B01/B08; 179 C y
referencia de 4096 muestras/400 rollovers, regresiones codec 5096/velocidad 11199.
Host/san 17/17 más refuerzo selector 1/1, ARM Debug 13/13 y fixture B10 Release;
Doxygen 32 funciones/USW. [Matriz/evidencia](../../openspec/changes/archive/2026-11-09-b10-seleccion-estable-modo/verify-report.md).
GPIO/base temporal/main y antirrebote eléctrico pendientes; sin duración TPI impuesta.

B11 propuesto (R01/R02/R08/R09/R10/R11/R15): soporte temporal para futuras
adquisiciones/comunicaciones, con SysTick y frecuencia explícita. Diseño de soporte,
no nuevo requisito de la consigna. Sin acuerdo, implementación ni evidencia nueva;
no acredita ventana de medición RPM, períodos de adquisición o precisión física.

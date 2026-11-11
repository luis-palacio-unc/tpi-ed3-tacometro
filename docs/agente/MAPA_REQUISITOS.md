# Mapa de requisitos

AP = anteproyecto.pdf §1; APA/APB = anexos A/B. G = guía REV 2.0.
U = instrucciones del usuario de incorporación SDD (2026-11-06).
Los IDs siguientes son locales de trazabilidad, no numeración de la consigna.
Las rutas indican módulos previstos; no acreditan implementación.

| ID / fuente | Requisito | Bloque | Archivos principales | Evidencia de aceptación |
|---|---|---|---|---|
| R01 AP | Ejecutar firmware en LPC1769/OM13085 | B00 aceptado: base de build | CMakeLists.txt, Aplicacion/main.c, cmake/, plataforma/ | S1/S3/S5/S6 PASS de build/inspección (verify-report B00); ejecución física pendiente |
| R02 AP/APA | Medir RPM con disco de 60 ranuras | B03/B03-R1 y B14 aceptados/archivados; conteo físico pendiente | Servicios/Medicion_Velocidad.*, Dispositivos/Sensor_Optico.* | B03-R1 S1–S10 PASS, 23 casos + oráculo 11199; revisión aceptada; señal real pendiente |
| R03 AP | Potenciómetro determina PWM en manual | B04/B08/B15 aceptados: normalización/mando/PWM directo; B16 aceptado/archivado para lectura; ADC/PWM físicos pendientes | Servicios/Mandos.*, Aplicacion/Procesamiento.*, Servicios/Driver_Motor.*, Dispositivos/Potenciometro.* | B04 S1–S7 PASS host/enlace ARM (verify-report archivado); ADC/PWM físicos pendientes |
| R04 AP | Regular RPM con consigna UART en automático | Por planificar | Aplicacion/Procesamiento.*, Servicios/Comunicacion.* | Ensayos de regulación y perturbación sobre motor; pendiente |
| R05 AP | Exclusión de fuentes de mando por modo | B01 aceptado; B10 aceptado/archivado para selector estable | Aplicacion/Procesamiento.*, Servicios/Mandos.* | B01 S1/S2 y B10 SE1–SE4 PASS lógico; integración física pendiente |
| R06 AP | Entrar en automático con PWM cero hasta nueva consigna válida | B01/B08/B10/B15 aceptados/archivados para transición y solicitud cero | Aplicacion/Procesamiento.*, Servicios/Comunicacion.*, Mandos.*, Driver_Motor.* | B01 S3–S6 y B10-S6 PASS lógico; recepción/actuación reales pendientes |
| R07 AP | Temperatura supera umbral: LED rojo y buzzer prioritarios | B05 corregido, aceptado y archivado: servicio genérico + LM35; B06 aceptado/archivado para decisión; B07 aceptado/archivado para coordinación; actuación física/adquisición pendientes | Servicios/Temperatura.*, Dispositivos/LM35.*, Driver_Alarmas.*, Aplicacion/Procesamiento.* | B05 S1–S8 PASS host/enlace ARM (verify-report archivado); B06 S1–S8 y 13 casos PASS; resultado lógico aceptado; alarma física pendiente |
| R08 AP | Adquirir potenciómetro/temperatura con ADC y DMA | B16 aceptado/archivado: getter de lectura almacenada; adquisición física pendiente | Dispositivos/LM35.*, Potenciometro.*, soporte/Adquisicion_ADC_DMA.* | B16 S1–S4 PASS para lectura de memoria; secuencia/canales, transferencia y coherencia sobre placa pendientes |
| R09 AP | UART recibe consignas y transmite RPM, temperatura y estado | B02 aceptado para recepción; B09 aceptado/archivado para codificación lógica TX; B13 aceptado para operaciones UART; integración/envío físico pendientes | Servicios/Comunicacion.*, Dispositivos/UART.* | B02 RX y B09 CT1–CT4 PASS lógico; UART/envío físicos pendientes |
| R10 AP/APB | Tres capas; Procesamiento integra control; acceso mediante servicios | Transversal | Aplicacion/, Servicios/, Dispositivos/ | Revisión de dependencias por bloque; pendiente |
| R15 U, aclaración de diseño | Aplicación y servicios independientes del hardware específico; adaptación del sensor en dispositivos (aclaración del usuario) | Transversal | Aplicacion/, Servicios/, Dispositivos/ | B05 servicio aislado/fuentes genéricas PASS; curva en LM35. B03-R1 recibe factor explícito sin configuración/dispositivos, aceptado/archivado |
| R11 G §4 | Documentar ESW/USW y diagramas | Transversal | .c/.h propios y futura documentación de entrega | B01–B10 aceptados; B11–B15 aceptados:42 funciones/USW; B16 aceptado:43, Doxygen estricto PASS; diagramas/resto pendiente |
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

B11-SysTick sustituido, sin aceptación (R01/R02/R08/R09/R10/R11/R15):
evidencia previa en [informe histórico](../../openspec/changes/b11-base-tiempos-lpc1769/verify-report.md),
corresponde sólo a db88286: host/san 20/20 y 205 C, ARM 15/15/Release y 40 USW.

B11 retirado por el usuario 2026-11-10. Timer.c/.h y Tiempo.c/.h eliminados;
TC1–TC5/S1–S8 describen una propuesta descartada, sin implementación actual.
Su evidencia en el cambio SDD y Git es histórica, no aceptación vigente.
R01/R02/R08/R09/R10/R11/R15 relacionados con temporización quedan por resolver;
definir juntos la responsabilidad del timer antes de especificar/implementar.

B11 wrapper Timer (R10/R15), aceptado/archivado2026-11-10:
WT1–WT3/S1–S3 vigentes en openspec/specs/timer/spec.md, implementación sólo Dispositivos/Timer.h/.c. USW17.1 init,
17.14 start,17.2 read. Drivers originales intactos; Temporizacion.h eliminado.
Evidencia: compilación host/ARM, dependencias exactas TIM_InitTimer/Enable/ReadTimer,
header aislado, overrides1/2/5 compilados y0 rechazado, Doxygen35. Regresiones
host17/17, ARM13/13. Operación temporal/enlace de biblioteca/placa no ejecutados.
[Matriz/evidencia](../../openspec/changes/archive/2026-11-10-b11-wrapper-timer/verify-report.md).

B12 de enlace/arranque pospuesto por usuario2026-11-10; no implementado.
B12 GPIO corregido aceptado/archivado2026-11-10 (R05/R07/R10/R11/R15): Selector/P2.6 y Buzzer/P2.7
activos altos por corrección del usuario; LED rojo/P0.22 bajo según esquema NXP.
DG1–DG3/S1–S4 corregidos por usuario: salidas void(bool), sin contexto/retorno.
Siete casos de polaridad/aislamiento/repetición; host/san18/18 y186 C; ARM13/13,
salidasO0/O2 y Doxygen38. Drivers/B07 originales intactos durante B12; composición resuelta en revisión R1 aceptada; aplicación/main pendientes.
[Evidencia](../../openspec/changes/archive/2026-11-10-b12-dispositivos-gpio/verify-report.md).
Manual UM10360 rev4.1 tabla84/p120 y cap9 revisados. Inicialización/enlace finales.

B07-R1 aceptado/archivado2026-11-10, contrato salidas-alarma sincronizado
(R07/R10/R11/R15): void(bool), dos funciones sin contextos; sólo despacho/argumento.
SA1–SA4/S1–S9 conformes:8 casos servicio+4 térmicos+8 GPIO; host/san18/18,183 C
y oráculo Python; ARM13/13/fixture Release; Doxygen38. B06/B12/Drivers intactos.
[Evidencia](../../openspec/changes/archive/2026-11-10-b07-r1-salidas-sin-contexto/verify-report.md).

B13 aceptado/archivado2026-11-10; contrato uart-dispositivo vigente
(R09/R10/R11/R15): UART_Read/Write, TX P0.2/RX P0.3 función01 asignados y
UART_BAUDRATE configurable. UD1–UD4/S1–S7:11 casos UART; host/san19/19,194 C
y oráculo; ARM13/13/O0/O2; Doxygen40. Parámetro/pinmux aún sin aplicar; integración/
ensayo físico pendientes, Drivers/servicios intactos.
[Evidencia](../../openspec/changes/archive/2026-11-10-b13-dispositivo-uart/verify-report.md).

B14 aceptado/archivado2026-11-10, contrato sensor-optico sincronizado
(R02/R04/R09/R10/R11/R15): lectura acumulada sin reset/RPM/ventanas. Sensor actual
XD-51 confirmado por usuario, sustituye HC-89; características eléctricas pendientes.
SO1–SO4/S1–S5 conformes:6 casos nuevos, host/san20/20,200 C+oráculo; ARM13/13/
objetoO0/O2 y Doxygen41. Timer/B03/Drivers intactos. Adquisición/integración físicas
pendientes. [Evidencia](../../openspec/changes/archive/2026-11-10-b14-sensor-optico-contador/verify-report.md).

B15 aceptado/archivado2026-11-10, contrato motor-dispositivo sincronizado
(R03/R06/R10/R11/R15): MR0=1000 requerido al integrar, mando directo0..1000 hacia
PWM_MATCH_1, sin cálculo/lectura MR0. MD1–MD4/S1–S5 conformes: cuatro casos,
1001 envíos exactos/64535 rechazos; host/san21/21,204 C+oráculo, ARM13/13/O0/O2,
Doxygen42. Modelo RAM retirado; Drivers/módulos/contratos aceptados intactos.
Configuración/frecuencia/potencia/PWM.1/adaptación/main físicos pendientes.
[Evidencia actual](../../openspec/changes/archive/2026-11-10-b15-dispositivo-motor-pwm/verify-report.md).

B16 corregido aceptado/archivado2026-11-11 (R03/R08/R10/R11/R15):
Pot_Read(void) devuelve RESULT0..4095 de ADC_DMA_Pot, definida en Potenciometro.c,
sin estados/validaciones. Extern interno para DMA; prueba enlaza sin soporte.
USW.2.1.12.1; canal AD0.0/P0.23 acordado. PD1–PD3/S1–S4 PASS para memoria:
3 casos B16, host/san22/22,207 C+oráculo, ARM13/13/O0/O2/propiedad de símbolo,
Doxygen43. Drivers90 originales/módulos/contratos aceptados intactos.
ADC/DMA/configuración/asociación de canales y enlace físico pendientes para el final.
[Evidencia](../../openspec/changes/archive/2026-11-11-b16-lectura-potenciometro-dma/verify-report.md).

Contrato vigente B16: openspec/specs/potenciometro-dma/spec.md.

B05-R1 con auditoría inicial/corrección H1/H2 verificadas; reauditoría/aceptación pendientes
(R07/R08/R09/R10/R11/R15): muestra propia, int32_t centésimas, redondeo B05,
sin estados de dispositivo. TE1/TE3/TE4 candidatos implementados; TE2 intacto.
S1/S2/S3/S6/S7/S8 PASS: host/san29/29,214 ejecuciones C,20480 comparaciones
exactas,8 probes, ARM13/13 y Doxygen43. m1 constantes derivadas; m2 test sin
soporte/símbolo único; m3 resumen portable sin enlaces externos rotos.
Corrección de auditoría: S1/oráculo ejercen todo bit fuera de15:4 (0xFFFF000F),
incluidos reservados23:16 y29:27; mutante sin máscara sobrevive antes y falla
después en unidad/oráculo. README describe centésimas directas. LM35.c intacto.
H3/ADDRn/configuración ADC/DMA/VREF y origen del estado genérico de fuente al final. Sin cierre/sincronización/archivo/push.
[Evidencia](../../openspec/changes/b05-r1-lectura-lm35/verify-report.md) y
[revisiones externas locales resumidas](../../openspec/changes/b05-r1-lectura-lm35/review-notes.md).

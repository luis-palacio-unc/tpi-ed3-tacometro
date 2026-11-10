# Contexto duradero

## Objetivo y fuentes

Tacómetro y control de motor DC con reductora; modos manual/automático y alarma
por temperatura ambiente. Entorno elegido por el usuario: CLion, lenguaje C.

Fuentes del TPI, situadas en el directorio padre del proyecto:
- `anteproyecto.pdf`, §1 y anexos A/B: comportamiento y módulos del tacómetro.
- `GUIA_TPI_EDIII.pdf`, REV 2.0: formato de entrega y documentación ESW/USW.

El anteproyecto define este sistema. Los diagramas de la guía son ejemplos, no
nuevos módulos obligatorios. La guía pide entrega `Informe/` y `Firmware/`;
se conservará la estructura de desarrollo y se resolverá el empaquetado al entregar.
La guía pide documentación @file/@brief/@details/@note e identificación ESW/USW;
indica no comentar el cuerpo de las funciones para la evaluación.

## Hardware y herramientas

- Especificado en anteproyecto: OM13085, LPC1769; LM35, sensor óptico HC-89,
  disco de 60 ranuras, potenciómetro de 10 kΩ, selector, LED rojo y buzzer activo.
- Confirmado por usuario: se usará CMSIS-DAP integrado.
- NXP confirma que OM13085 incorpora LPC1769 y CMSIS-DAP. No asumir que la
  revisión D del esquema publicado coincide con la placa física del usuario.
- LPC1769: Cortex-M3; la SRAM tiene bancos separados, no una región contigua de
  64 kB. Direcciones y accesibilidad DMA se revisarán al diseñar linker/adquisición.
- Instalados y verificados: CMake 3.28.3, GCC host 13.3.0, GCC ARM 13.2.1,
  binutils ARM, Git y gh. B00 agrega perfiles CMake host y ARM, sin dependencias nuevas.
- Biblioteca existente: CMSISv2p00_LPC17xx y drivers modificados; historia CMSIS
  2011/2014, procedencia de modificaciones sin fijar por commit. Preservar licencias.
- `system_LPC17xx.c` contiene configuración activa de PLL, relojes y flash;
  su incorporación requiere revisión, no ejecutarlo como inicialización asumida.

## Arquitectura y decisiones

- Aplicacion: main y Procesamiento; Servicios: Medicion_Velocidad, Temperatura,
  Mandos, Driver_Alarmas, Comunicacion y Driver_Motor; Dispositivos: los 8 del anexo B.
  Tiempo es el wrapper candidato agregado en B11, extension local del servicio temporal.
- Aplicación accede a dispositivos sólo mediante servicios. Procesamiento integra
  modos y regulación; Driver_Motor es el intermediario para accionar el motor.
- Soporte compartido aceptado por el usuario: ADC/DMA y base de tiempos, dentro
  de Dispositivos. No es una cuarta capa.
- Aclaración del usuario: el profesor exige separación de abstracciones y portabilidad
  de la aplicación entre configuraciones físicas. La aplicación consume magnitudes
  y estados lógicos; los dispositivos resuelven registros, pines y captura, y los
  servicios realizan la adaptación. Aclaración posterior: los servicios también deben
  independizarse del hardware específico; la curva/calibración del sensor pertenece
  al dispositivo. Generalizar sólo lo que resuelva requisitos concretos.
  Explicar las decisiones en lenguaje sencillo durante la revisión de diseño posterior.
- Se propuso lazo cooperativo e interrupciones breves; se detallará en diseños SDD.
- Entrada en automático exige PWM cero y nueva consigna válida; no reutilizar la
  anterior ni comandos pendientes anteriores al cambio de modo.
- Alarma de temperatura prioritaria, visual y acústica. No se ha requerido detener
  el motor por temperatura ni se ha acordado aún histéresis/enclavamiento.
- Consignas aceptadas: 0..185 RPM inclusive; máximo aclarado por el usuario.
- No se ha elegido PI/PID, pines o frecuencias; recepción ASCII B02 y formato lógico B09 acordados.

## SDD y Git

Persistencia por archivos, sin instalar frameworks. `openspec/specs/` contiene sólo
contratos aceptados: build (B00), modos (B01), comunicación (B02/B09), medición
(B03/B03-R1), mandos (B04/B10), temperatura (B05), alarma lógica (B06), salidas de
alarma (B07) y mando de motor (B08). B00–B10 y B03-R1 aceptados y archivados.
Contrato de temperatura corregido vigente desde
2026-11-07. Revisión independiente B03-R1 aceptada y archivada el 2026-11-09; contrato sincronizado.
GitHub público, verificado el 2026-11-09: https://github.com/luis-palacio-unc/tpi-ed3-tacometro
Git local inicializado en main por acuerdo del usuario; origin se creó privado.
El cambio bd7bc0b de Luis actualiza esa visibilidad en README. El usuario
autorizó integrarlo en main y completar la publicación B10, conservando historia.
El usuario autorizó commit y push de la estructura inicial al aceptar B00.
El usuario autorizó después commits al finalizar cada bloque acordado. El push sigue
requiriendo autorización independiente; el usuario autorizó publicar los commits B01
y su cierre, y posteriormente B02 y su cierre. También autorizó publicar B03
y su cierre al aceptarlo, y luego B04 y su cierre. El usuario autorizó publicar B05, su corrección y cierre al aceptarlo el 2026-11-07.
Al aceptar B06 el 2026-11-09, autorizó publicar los commits pendientes
(B03-R1, cierre/propuesta y B06) junto con el cierre B06.
Al aceptar B07 el 2026-11-09, autorizó publicar los commits pendientes, su cierre
y preparar la propuesta B08. Al aceptar B08 el 2026-11-09, autorizó publicar su commit
y el cierre y preparar la propuesta B09. Al aceptar B09 el 2026-11-09, autorizó
publicar su commit/cierre y proponer B10. Al aceptar B10 el 2026-11-09, autorizó
publicar su commit/cierre y proponer B11. Publicaciones futuras requieren autorización independiente.
Se conserva configuración de firma Git,
actualmente sin firma automática habilitada.

## Referencias de fabricante consultadas

- [OM13085](https://www.nxp.com/design/design-center/development-boards-and-designs/OM13085).
- [Datasheet LPC176x, rev. 9.10](https://www.nxp.com/docs/en/data-sheet/LPC1769_68_67_66_65_64_63.pdf).
- [Erratas LPC176x, rev. 10.6](https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf).
- [UM10360 rev.2, copia del manual NXP en Keil](https://www.keil.com/dd/docs/datashts/philips/lpc17xx_um.pdf):
  cap.2/32 revisados para B00. La rev.3.1 publicada en NXP requiere cuenta; revisar
  documentación/erratas pertinentes antes de implementar periféricos. Para proponer
  B11 se revisaron capítulos 23/34 (SysTick) y erratas 10.6 el 2026-11-09;
  no acredita precisión temporal ni configuración real del reloj.

B05 usa la transferencia nominal del
[LM35 TI, §7.3.1](https://www.ti.com/lit/ds/symlink/lm35.pdf), consultada para separar
conversión lógica y precisión eléctrica. Referencia real, variante y montaje pendientes;
B05 no configura adquisición ni fija umbral de alarma.

## Plataforma B00

Build explícito Cortex-M3/Thumb, main mínimo, startup propio y linker por bancos.
.data/.bss en SRAM local; pila reservada de 2 kB, sin heap. AHB0/AHB1 sin asignar.
Checksum de boot calculado al enlazar; palabra CRP 0xffffffff reservada.
No se configura reloj/PLL ni se llama SystemInit. No asumir el reloj heredado
del boot ROM/ISP. Las verificaciones son estáticas, sin aceptación física de arranque.

## Decisiones B01 e integración futura

Procesamiento expone Inicializar/Actualizar con contexto explícito, sin hardware ni
estado global. Fuente CERO/MANUAL/AUTOMATICA única; manual entero 0..1000 por mil.
La fuente automática entrega consigna para un controlador posterior; no calcula PWM.
Procesamiento descarta consignas >185 RPM aunque valida sea true; no las satura ni
reemplaza la activa. Este límite de aceptación no recorta las mediciones reales.
Órdenes uint64_t monotónicas sin rollover, asignadas por el integrador al recibir
(eventos no traen esa autoridad desde PC), preservadas en cola. Captura y llamadas
requieren serialización externa; integrar esto antes de habilitar recepción por ISR.
Entrada automática establece barrera; consigna en la misma actualización no habilita.
Reiniciar numeración requiere inicializar y purgar colas. API completa en Procesamiento.h.

Documentar cada nueva ESW/USW con @file/@brief/@details/@note, IDs, parámetros, retornos,
unidades, errores y concurrencia. Sin comentarios dentro de cuerpos de producción,
según guía del TPI. Doxyfile parcial estricto; ampliar INPUT con cada módulo documentado.
Doxygen instalado genera HTML/XML juntos; contenido español y etiquetas automáticas
inglesas. No se instala ni actualiza Doxygen para generar la documentación final.

## Recepción lógica B02

Comunicacion expone Inicializar/RecibirByte/ExtraerConsigna, con eventos propios del
servicio. ASCII RPM + espacio + 1..3 dígitos + LF/CRLF; ceros iniciales permitidos.
0..185 RPM, definidos una sola vez en Configuracion/Limites.h. Procesamiento conserva
su alias; no hay dependencia Servicios→Aplicacion. No se limita la medición real.
Buffer 8 bytes sin LF y cola 4 eventos. Trama excesiva descarta hasta LF; cola llena
rechaza nuevo con error observable, sin sobrescribir FIFO. La marca del evento es la
secuencia global capturada al recibir LF; cada byte exige orden único creciente.
Captura/recepción/extracción deben serializarse externamente antes de integrar UART/ISR.
No hay transmisión, temporización física o main integrado en B02. API y errores en
Comunicacion.h; contrato vigente de comunicación y evidencia en archivo B02.

## Medición lógica B03 y revisión R1

B03 original aceptado/archivado fija 60 pulsos/vuelta; ese historial es inmutable.
Usuario autorizó revisarlo el 2026-11-07 bajo R15 y aceptó el resultado el
2026-11-09. B03-R1 archivado; contrato vigente de medición actualizado por requisitos.
Evidencia en openspec/changes/archive/2026-11-09-b03-factor-pulsos-configurable/.

API actual Medicion_Velocidad_Actualizar recibe pulsos, duracion_us y factor
pulsos_por_vuelta uint32_t explícitos por llamada. Servicio no incluye configuración
física; Configuracion/Medicion.h conserva TPI_MEDICION_PULSOS_POR_VUELTA=60 para
un flanco/ranura del TPI. Factor es relación abstracta conteos/vuelta; no introduce
modelo de sensor, placa, registros ni ganancia eléctrica en el servicio.
Salida 8 bytes ARM: milésimas RPM uint32_t y estado. Sin recorte de 185 RPM.
Prioridad NULL/duración cero/factor cero/rango; error escribible limpia, válido recupera.
Inicializar mantiene SIN_DATOS/cero. Sólo VALIDA habilita consumo, incluido cero.

Redondeo cercano/empate arriba sin pérdida intermedia; fórmula general por cociente,
resto y seis pasos binarios uint64_t evita numerador mayor que 64 bits.
SEGUNDOS_POR_MINUTO=60 es conversión universal de tiempo, distinta del factor físico.
libgcc existente, sin libc/heap/floats/globals. Ventana/configuración coherentes,
overflow previo y serialización quedan externos. Sin captura/timeout/main ni hardware.

## Normalización manual B04

Mandos (ESW.2.1.5) recibe lectura y fondo de escala uint32_t explícitos; salida
uint16_t 0..1000 por mil y estado. Cercano/empate arriba, error aritmético <=0,5
por mil; independiente de resolución, VREF y canal. TPI_MANDO_MANUAL_MAX=1000 vive
en Limites.h; aliases públicos de Mandos y B01 conservan el mismo rango.
Inicializar deja SIN_DATOS/cero; NormalizarManual reemplaza toda salida sin leer
contenido anterior. Errores NULL/escala cero/lectura excesiva en ese orden; salida
escribible inválida queda a cero, recuperable con siguiente dato válido. Sólo consumir
VALIDO. Captura coherente, validación del ADC y serialización son responsabilidad
del integrador; sin timeout. No implementa selector, ADC/DMA ni PWM, ni integra main.

## Temperatura B05 corregido

La curva LM35 y parámetros ADC pertenecen a Dispositivos/LM35 (ESW.2.1.11), no al
servicio. Fuente local confirmada por usuario: /home/luis/Desktop/Universidad/ed3/Datasheet_LM35_TI.pdf,
TI SNIS159H §7.3.1 p.13, ganancia nominal 10 mV/°C. Variante/montaje/VREF reales pendientes.
LM35 inicializa un registro y convierte muestra/escala/referencia uint32_t a Celsius
uint32_t, con estados/errores propios. Escala son cuentas por referencia; muestra<escala.
Modelo sin offset, mismo redondeo y cotas verificados en B05 inicial; sin ADC/DMA físico.

Servicios/Temperatura (ESW.2.1.4) recibe int32_t centésimas Celsius y estado de fuente
genérico válido/sin datos/error; conserva magnitud o invalida con cero. NULL/enum
desconocido son error de argumento; nueva fuente válida recupera. No incluye ni
enlaza LM35, no interpreta ADC, VREF, ganancia o rango de un sensor. Admite negativos
de otra fuente; no adquiere temperaturas negativas mediante LM35.
Contratos propios evitan dependencia del dispositivo hacia el servicio. Adaptación
de estados/cota se demuestra en pruebas; composición real pendiente, sin acceso
directo a dispositivos desde lógica de aplicación. No añadir callbacks hasta necesitar adquisición.
B05 no implementa frescura, decision de alarma ni main; acceso serializado externo.

R15: corrección B05 y revisión B03-R1 aceptadas; no afirmar implementación
física completa de toda la capa de servicios. La adquisición y composición real siguen pendientes.

## Alarma lógica B06

Resultado B06 aceptado y archivado el 2026-11-09; contrato vigente sincronizado. Umbral configurable int32_t en centésimas Celsius explícito, sin default.
Procesamiento_EvaluarAlarmaTemperatura (USW.2.1.2.7) consume const Temperatura_Lectura*;
retorna ACTIVA sólo con VALIDA y T>umbral, INACTIVA con T<=umbral. Ausencia,
fallo de fuente y error de argumento son estados distintos, nunca temperatura normal.
NULL/enum desconocido producen ERROR_ARGUMENTO. No retiene decisión ni cambia B01.
Consumidor asigna el retorno completo; sólo ACTIVA/INACTIVA son evaluaciones disponibles.

Aplicación incluye el contrato del servicio; no duplica enum de fuente ni conoce
LM35/ADC. No requiere estructura de salida/contexto o inicialización nuevos.
Serializar lectura del registro: const no da atomicidad ni frescura. Se preservan
los seis cuerpos/tipos B01; fixture B01 agrega únicamente ruta de encabezados Servicios.
B07 coordina solicitudes mediante proveedores externos; señalización física/prioridad temporal,
histéresis, timeout, enclavamiento y respuesta física a fallo siguen pendientes.
No se ha acordado parar el motor por temperatura. Evidencia y cierre B06 en archivo; contrato de alarma-temperatura vigente.

## Coordinación lógica de salidas B07 y revisión R1

B07 original aceptado/archivado2026-11-09; B07-R1 aceptado/archivado2026-11-10.
Contrato salidas-alarma sincronizado, historial conservado. Llamada desde aplicación/
main explícitamente pendiente para integración final, según usuario.
Código actual: Driver_Alarmas_Aplicar (USW.2.1.6.1) recibe interfaz const de dos
funciones void(bool), sin contextos. Valida ambas antes de llamar LED→buzzer una
vez cada uno con el mismo activar; repeticiones también se despachan.
Sólo ACEPTADA/ERROR_ARGUMENTO: despacho completo o interfaz NULL/incompleta sin
llamadas. Se eliminan errores de proveedor porque esas funciones no dan diagnóstico.

Servicio sin dependencia de aplicación, sensores, GPIO, dispositivos o MCU; sin
init/heap/estado global/esperas/reintentos. Interfaz/funciones estables y serialización
externas. La duración/pila total dependen de salidas; ACEPTADA no confirma electricidad.
Composición B12 probada en host con funciones reales y doble GPIO, sin adaptadores/
casts; no integra main. Adaptación B06 externa preserva diagnóstico ante ausencia/
error sin emitir orden, ni define respuesta física. Contrato vigente salidas-alarma
refleja R1; no usar archivo B07 original como código actual.

## Documentación final y B08

Pedido del usuario 2026-11-09: una vez implementados todos los ESW, agregar al
informe diagramas de secuencia pequeños que expliquen cómo cooperan aplicación,
servicios y dispositivos, mostrando llamadas y retornos con lenguaje sencillo.
Contrastar con implementación aceptada; no elaborarlos todavía ni tratarlos como prueba física.
B08 aceptado/archivado el 2026-11-09; contrato mando-motor vigente sincronizado.
Driver_Motor (ESW.2.1.8) expone Aplicar(uint16_t por_mil) y SolicitarCero con interfaz
const de función bool(void*,uint16_t) y contexto opaco externo; dos USW .1/.2.
Límite 1000 compartido en Limites.h; validar argumento antes de rango, rechazar sin
envío/saturación, informar fallo del proveedor y recuperar en siguiente solicitud.
Cero explícito solicita 0; no confirma detención. Sin init/heap/globals/esperas/reintentos.
Interfaz/contextos vivos/estables/serializados; proveedor puede modificar su propio
estado, debe retornar. Aceptación no es confirmación física; validar orígenes más
amplios/con signo antes de convertir. Adaptación B04/B01 sólo en pruebas/fixture,
sin tocar main: MANUAL válido, CERO y AUTOMATICA para futuro regulador, nunca RPM→PWM.
Cierre/publicación B08 autorizados al aceptar resultado.

## Telemetría lógica B09

B09 aceptado/archivado el 2026-11-09. Contrato comunicación sincronizado: conserva
B02-R1–R4, amplía B02-R5 y agrega CT1–CT4. Publicación del cierre autorizada.
Comunicacion_CodificarTelemetria (USW.2.1.7.7) recibe instantánea const y enums propios,
buffer/capacidad y longitud. La aplicación reunirá magnitudes B03/B05 y estado B01;
adaptación sólo en pruebas/fixture por ahora, sin cambiar Procesamiento o main.
Formato ASCII RPM=valor;TEMP=valor;ESTADO=MANUAL/AUTO_ESPERA/AUTO_ACTIVO, terminado LF.
RPM conserva tres decimales y temperatura dos/signo; ND/ERR distintos de cero.
Máximo 53 bytes de trama, 54 con NUL; longitud excluye NUL. Punteros/enums/capacidad
en esa prioridad; error limpia longitud y primer byte si escribibles, sin otros bytes
modificados. Recuperable, sin estado propio ni init RX. Datos/destinos no solapados,
vivos/coherentes/serializados. Sin heap/flotantes/libc nueva/UART/esperas/recorte 185.
Helpers USW.2.1.7.8/.9/.10 y cota de construcción documentados; INT32_MIN usa magnitud
unsigned definida. Codificación no acredita transmisión PC ni frescura física.
RX B02 preservado; UART/envío/periodicidad/main y aviso térmico opcional fuera de B09.

## Selector lógico B10

Resultado aceptado/archivado el 2026-11-09; publicación del cierre autorizada.
Contrato mandos sincronizado: MA3/S6 modificado y SE1–SE4 agregados, conservando
MA1/MA2/MA4. Mandos agrega tres USW .2.1.5.3/.4/.5.
Mandos_Selector contiene salida estado/modo y metadatos internos por instancia.
InicializarSelector configura intervalo 0..INT32_MAX ms sin default TPI, deja
SIN_DATOS/NINGUNO. ActualizarSelector recibe modo/fuente y timestamp uint32_t ms;
confirma en el intervalo exacto, cada cambio reinicia candidato, confirmado se
conserva durante otro candidato pendiente. Cero inmediato. Sólo VALIDO permite consumo.
Ausencia/fallo/error invalida confirmación/candidato. Prioridad: NULL, configuración,
fuente desconocida, tiempo, disponibilidad y modo. Fuente desconocida no registra
tiempo; ausencia/fallo/modo inválido conservan marca aceptada. Error temporal descarta
anclaje y permite época nueva con recalificación completa. Configuración excesiva
requiere reinicio válido. Resta modular, marcas iguales/wrap y avances <2^31 ms;
cadencia, captura y serialización externas, sin detectar vueltas completas omitidas
o rebotes entre muestras. Sin GPIO, reloj, heap, esperas o globals mutables.
Adaptación B01/B08 sólo en pruebas/fixture; órdenes uint64_t B01 no son los ms.
Rebotes no cambian modo de aplicación; entrada confirmada manual→auto exige cero y
consigna posterior. Indisponibilidad no emite órdenes desde adaptador de prueba;
política física y reentrada tras fallo, GPIO/main quedan pendientes. B04/B01/B08 intactos.

## B11: wrappers del driver original — 2026-11-10

Diseños SysTick, TC/puerto y servicio Tiempo retirados; no retomarlos. Usuario
restituyó Drivers:90 archivos iguales a a21cc1e/origin/main. Prohibido modificarlos.
B11 aceptado/archivado2026-11-10: Dispositivos/Timer.c/.h con sólo
Timer_Init→TIM_InitTimer, Timer_Start→TIM_Enable y Timer_Read→TIM_ReadTimer.
TIMER0 sólo en .c; header sin CMSIS. TIMER_BASE_MS default1, configurable al
compilar en Timer.h; Read normaliza a ms uint32 modular. Temporizacion.h eliminado.
Sin estado mutable/ISR/acumulador/proveedor/puerto/servicio Tiempo. ESW17 y USW
.17.1/.2 conservan init/lectura; .17.14 arranque nuevo, IDs retirados no reciclados.

Init deja detenido, luego Start; timer exclusivo desde reset sin match/captura
previos y SystemCoreClock correcto. Base/PCLK representables/exactos según driver;
no agregar configuración propia de reloj. APIs serializadas; reinit exige marcas
nuevas, intervalos reales<2^31ms. Main, servicios aceptados y drivers intactos.
El wrapper se compila con encabezados originales; no hay nueva prueba que repita
las tres llamadas. Enlace de biblioteca original/arranque y prueba física pendientes
de integración; no declarar funcionamiento eléctrico por compilación.

Nueva instrucción2026-11-10: inicialización de periféricos, enlace e integración
global se dejan para el final. Completar primero elementos de Dispositivos mediante
wrappers de drivers originales; prohibición de modificar biblioteca vigente.
La propuesta b12-integracion-arranque-timer está pospuesta, no aceptada.
B12 corregido aceptado/archivado2026-11-10; contrato dispositivos-gpio vigente.
Usuario corrige selector/buzzer a activos por alto, pines P2 sin funciones EINTx.
Elegidos selectorP2.6 y buzzerP2.7; P2.10–13 reservados EINT0–3, P2.0–5 libres.
LED rojoP0.22 activo bajo según esquema OM13085 revD; revisión física pendiente.
Manual aportado: UM10360 rev4.1 (2016-12-19), tabla84/p120 y cap9/p132/150 revisados.
Lectura/accionamiento sólo; multiplexado/dirección/polarización/FIOMASK y arranque finales.

Dispositivos interactúa directamente con el hardware usando los drivers disponibles;
no requiere transparencia respecto del MCU dentro de esa capa ni duplicación de lógica
de la biblioteca. Mantener independencia y contratos lógicos en Aplicación/Servicios.

B12 implementa sólo Selector_Read (GPIO_ReadValue/P2.6 alto), LED_Write
(GPIO_SetPinState/P0.22 bajo) y Buzzer_Write (GPIO_SetPinState/P2.7 alto).
Corrección autorizada2026-11-10: ambas salidas void(bool activar), sin contexto
ni retorno ficticio. B07-R1 ya implementa dos funciones void(bool), sin contextos,
y ACEPTADA/ERROR_ARGUMENTO para despacho/validación. Composición directa B12
verificada en host, R1 aceptado. Sin adaptadores/casts; no hay conexión desde main.
Tres USW.13.1/.14.1/.15.1 documentadas. Sin estado mutable ni nuevos servicios.
Inicialización/enlace y uso desde main se mantienen para integración final.

B13 propuesto, sin acuerdo/implementación: dispositivo UART con dos transferencias
no bloqueantes, buffers externos y conteo real de bytes; UART0 propuesto, no asignado
por acuerdo todavía. Reutilizar driver original. Configuración/FIFO/DLAB/pines y
exclusividad previas al uso físico; baudios/cableado, integración y diagnóstico de
línea pendientes. No añadir init/colas/ISR ni modificar servicios en ese bloque.

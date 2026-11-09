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
- No se ha elegido PI/PID, pines, frecuencias ni telemetría; recepción lógica ASCII en B02.

## SDD y Git

Persistencia por archivos, sin instalar frameworks. `openspec/specs/` contiene sólo
contratos aceptados: build (B00), modos (B01), comunicación (B02), medición
(B03/B03-R1), mandos (B04) y temperatura (B05). B00–B05 aceptados y archivados.
Contrato de temperatura corregido vigente desde
2026-11-07. Revisión independiente B03-R1 aceptada y archivada el 2026-11-09; contrato sincronizado.
GitHub privado: https://github.com/luis-palacio-unc/tpi-ed3-tacometro
Git local inicializado en main por acuerdo del usuario, con origin privado.
El usuario autorizó commit y push de la estructura inicial al aceptar B00.
El usuario autorizó después commits al finalizar cada bloque acordado. El push sigue
requiriendo autorización independiente; el usuario autorizó publicar los commits B01
y su cierre, y posteriormente B02 y su cierre. También autorizó publicar B03
y su cierre al aceptarlo, y luego B04 y su cierre. El usuario autorizó publicar B05, su corrección y cierre al aceptarlo el 2026-11-07.
Nuevas publicaciones requieren autorización independiente.
Se conserva configuración de firma Git,
actualmente sin firma automática habilitada.

## Referencias de fabricante consultadas

- [OM13085](https://www.nxp.com/design/design-center/development-boards-and-designs/OM13085).
- [Datasheet LPC176x, rev. 9.10](https://www.nxp.com/docs/en/data-sheet/LPC1769_68_67_66_65_64_63.pdf).
- [Erratas LPC176x, rev. 10.6](https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf).
- [UM10360 rev.2, copia del manual NXP en Keil](https://www.keil.com/dd/docs/datashts/philips/lpc17xx_um.pdf):
  cap.2/32 revisados para B00. La rev.3.1 publicada en NXP requiere cuenta; revisar
  documentación/erratas pertinentes antes de implementar periféricos.

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
Acceso serializado, sin frescura/alarma/main implementados.

R15: corrección B05 y revisión B03-R1 aceptadas; no afirmar implementación
física completa de toda la capa de servicios. La adquisición y composición real siguen pendientes.

## Alarma: configuración y propuesta B06

Usuario confirmó el 2026-11-09 que el umbral será parámetro de configuración,
sin valor fijo acordado. Propuesta B06: decisión lógica en Procesamiento con
temperatura física/validez y umbral explícitos; servicio Driver_Alarmas aplicará
solicitudes al integrar señalización. Alcance B06 pendiente de acuerdo.
Anteproyecto pide superar el umbral, no fija número, histéresis ni respuesta física
ante fallo de adquisición. No interpretar esa omisión como autorización para
parar motor o inventar un patrón LED/buzzer. Prioridad temporal pendiente de integración.

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
- Se propuso lazo cooperativo e interrupciones breves; se detallará en diseños SDD.
- Entrada en automático exige PWM cero y nueva consigna válida; no reutilizar la
  anterior ni comandos pendientes anteriores al cambio de modo.
- Alarma de temperatura prioritaria, visual y acústica. No se ha requerido detener
  el motor por temperatura ni se ha acordado aún histéresis/enclavamiento.
- No se ha elegido PI/PID, pines, frecuencias, rangos o protocolo de comunicación.

## SDD y Git

Persistencia por archivos, sin instalar frameworks. `openspec/specs/` contiene sólo
contratos aceptados; dominio build vigente desde el cierre de B00.
Cambios en `openspec/changes/`; B01 implementado/verificado, resultado pendiente de aceptación.
GitHub privado: https://github.com/luis-palacio-unc/tpi-ed3-tacometro
Git local inicializado en main por acuerdo del usuario, con origin privado.
El usuario autorizó commit y push de la estructura inicial al aceptar B00.
El usuario autorizó después commits al finalizar cada bloque acordado. El push sigue
requiriendo autorización independiente. Se conserva configuración de firma Git,
actualmente sin firma automática habilitada.

## Referencias de fabricante consultadas

- [OM13085](https://www.nxp.com/design/design-center/development-boards-and-designs/OM13085).
- [Datasheet LPC176x, rev. 9.10](https://www.nxp.com/docs/en/data-sheet/LPC1769_68_67_66_65_64_63.pdf).
- [Erratas LPC176x, rev. 10.6](https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf).
- [UM10360 rev.2, copia del manual NXP en Keil](https://www.keil.com/dd/docs/datashts/philips/lpc17xx_um.pdf):
  cap.2/32 revisados para B00. La rev.3.1 publicada en NXP requiere cuenta; revisar
  documentación/erratas pertinentes antes de implementar periféricos.

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

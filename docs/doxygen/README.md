# Documentación incremental de firmware

Desde la raíz: `doxygen docs/doxygen/Doxyfile` (Doxygen 1.9.8 verificado).
Salida parcial HTML/XML en build/doxygen/, excluida de Git. No instalar dependencias
para generar documentación sin autorización.

- Usar @file, @brief, @details y @note ESW.x.x.x por módulo.
- Cada función/unidad tiene ID USW.x.x.x.x, incluidos helpers internos.
- Documentar parámetros/dirección, retorno, unidades, rangos, errores, efectos,
  precondiciones y concurrencia. Tipos, miembros y constantes también documentados.
- Mantener comentarios de contrato fuera del cuerpo de funciones de producción,
  como pide la guía del TPI.
- Ampliar INPUT al desarrollar módulos; la configuración actual cubre main, Procesamiento, Comunicacion, Medicion_Velocidad, Mandos, Temperatura, Driver_Alarmas, Driver_Motor, LM35, Limites, Configuracion/Medicion.h y Timer, Tiempo y Temporizacion.h. No usar EXTRACT_ALL para ocultar falta de documentación.
- Conservar advertencias como errores. HTML/XML juntos permiten comprobar @return
  con la versión instalada; las etiquetas automáticas son inglesas y el contenido español.

Validación actual B11 TC/Tiempo: 38 funciones/USW en XML (7 Procesamiento,
10 Comunicacion, 2 Medicion_Velocidad, 5 Mandos, 2 Temperatura, 2 LM35,
Driver_Alarmas, 2 Driver_Motor, cuatro Timer, dos Tiempo y main), sin avisos.
USW.2.1.4.1/.2 del servicio genérico; USW.2.1.11.1/.2 del dispositivo.
Documentar unidades físicas/estados en el servicio y conversión/errores específicos
en LM35. La inclusión de archivos del dispositivo no introduce dependencia del servicio.
La sonda sin @return documentada en B01 sigue como evidencia del control estricto.
El modo sólo XML instalado generó un aviso falso, reproducido incluso apareciendo
el retorno en el XML. No se suprimió la comprobación de retornos.

Referencia: [bloques documentales](https://www.doxygen.nl/manual/docblocks.html) y
[configuración oficial](https://www.doxygen.nl/manual/config.html).

B03-R1 conserva sus dos USW: API de velocidad con factor explícito, prioridad
de errores, unidades y aritmética acotada documentadas en USW.2.1.3.1/.2.
Configuracion/Medicion.h documenta el factor físico del TPI fuera del servicio.
Doxygen estricto HTML/XML pasa; no agrega funciones de producción.

B06 agrega USW.2.1.2.7: decisión por temperatura/validez del servicio y umbral
configurable; enum completo, unidades, argumentos, ausencia/error y concurrencia
documentados. Doxyfile ya cubre los archivos; no se cambia ni desactiva controles.

B07 agrega ESW.2.1.6/USW.2.1.6.1: interfaz/callbacks/contextos, estados, validación
previa y fallos parciales documentados. Doxyfile amplía INPUT con Driver_Alarmas.h/.c,
sin modificar controles estrictos. Proveedores emulados de tests no son unidades de
dispositivos de producción ni acreditan estado eléctrico.

El plan de diagramas de secuencia para el informe final queda en
[CONTEXTO](../agente/CONTEXTO.md#documentación-final-y-b08); elaboración diferida hasta completar los ESW.

B08 agrega ESW.2.1.8/USW.2.1.8.1/.2: mando por mil, solicitud explícita de cero,
interfaz/proveedor/contexto opacos, validación/rango, errores/recuperación y concurrencia.
Doxyfile incluye Driver_Motor.h/.c; HTML/XML estricto pasa sin avisos. Las pruebas
de composición no documentan PWM físico como implementado.

B09 agrega USW.2.1.7.7/.8/.9/.10: API de codificación, literales internos, entero y
fracción fija. Instantánea/enums, unidades, cota de buffer, longitud, limpieza/prioridad
de errores y no solapamiento documentados. Doxyfile ya incluye Comunicacion.h/.c;
HTML/XML estricto pasa, sin avisos ni cambios de controles. snprintf sólo aparece en pruebas host.

B10 agrega USW.2.1.5.3/.4/.5: inicializar selector, actualizar estabilidad y helper
privado de invalidación. Documenta contexto, fuente/modo/estado, intervalo/tiempo
modular, errores/recuperación, cadencia y concurrencia. Doxyfile ya incluye Mandos;
HTML/XML estricto pasa sin avisos. Las dos USW B04 conservan sus contratos.

B11 TC conserva USW.2.1.17.1/.2/.3/.9 en Timer_Init/Read/Stop/Source.
Retiradas .4–.8 SysTick y .10–.13 del puerto eliminado; no reciclar identificadores.
Tiempo ESW.2.1.18 con USW .18.1/.2 permanece sin cambios y sin dependencia de MCU.
Estas extensiones locales no son módulos impuestos por anexo B (.1–.16).
Doxyfile incluye Timer.h/.c, Tiempo.h/.c y Temporizacion.h; 38 funciones propias
únicas, seis TC/Tiempo, HTML/XML sin avisos ni controles relajados. El nuevo contrato
TIM_InitFreeRunning se documenta en su encabezado de biblioteca y se revisa aparte;
las funciones de la biblioteca no se cuentan como nuevas USW del TPI.

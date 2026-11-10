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
- Ampliar INPUT al desarrollar módulos; la configuración actual cubre main, Procesamiento, Comunicacion, Medicion_Velocidad, Mandos, Temperatura, Driver_Alarmas, Driver_Motor, LM35, Limites, Configuracion/Medicion.h, Timer, Selector, LED, Buzzer y UART. No usar EXTRACT_ALL para ocultar falta de documentación.
- Conservar advertencias como errores. HTML/XML juntos permiten comprobar @return
  con la versión instalada; las etiquetas automáticas son inglesas y el contenido español.

Validación actual B13 UART:40 funciones/USW en XML (7 Procesamiento,
10 Comunicacion, 2 Medicion_Velocidad, 5 Mandos, 2 Temperatura, 2 LM35,
Driver_Alarmas, 2 Driver_Motor, tres Timer, Selector, LED, Buzzer, dos UART y main), sin avisos.
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

Antecedente B11 retirado: se eliminaron Timer/Tiempo de INPUT y la documentación
volvió al alcance B00–B10/B03-R1 (32 funciones). Contratos del diseño temporal
retirado quedan sólo como antecedentes; no acreditar módulos eliminados como actuales.

Nuevo B11 acotado: INPUT incluye Dispositivos/Timer.h/.c. USW.2.1.17.1 init,
.17.2 lectura y .17.14 arranque; tres wrappers de biblioteca original, sin helpers.
Header documenta base/unidades, precondiciones, épocas y modularidad. Doxygen35
funciones propias únicas, HTML/XML sin avisos. Temporizacion.h y Tiempo no existen.

B12 incluye Selector/LED/Buzzer.h/.c con USW.2.1.13.1/.14.1/.15.1. Documenta
pines/niveles, delegación a biblioteca, configuración previa y serialización.
Corrección: LED_Write/Buzzer_Write void(bool activar), sin contexto ni @return.
B07-R1 documenta su nueva interfaz void(bool); composición directa verificada en host.
HTML/XML estricto38 funciones propias únicas, sin avisos ni controles relajados.

B07-R1 conserva ESW.2.1.6/USW.2.1.6.1: dos funciones void(bool) sin contextos,
enum ACEPTADA/ERROR_ARGUMENTO, validación completa previa/orden/retornos y acceso
serializado documentados. Sin diagnósticos físicos de salida. Doxyfile ya cubre el
servicio; HTML/XML estricto38 funciones sin avisos, sin relajar controles.

B13 agrega ESW.2.1.16/USW.2.1.16.1/.2: UART_Read/Write y UART_BAUDRATE documentados.
INPUT incluye UART.h/.c. P0.2 TX/P0.3 RX, modo no bloqueante, cantidades/bytes,
NULL/cero, progreso parcial, vida/serialización y configuración previa documentados.
Parámetro115200 modificable, no aplicado al hardware hasta inicialización final.
Doxygen HTML/XML40 funciones sin avisos; no se relajan controles.

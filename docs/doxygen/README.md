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
- Ampliar INPUT al desarrollar módulos; la configuración actual cubre main, Procesamiento, Comunicacion, Medicion_Velocidad, Mandos y Limites. No usar EXTRACT_ALL para ocultar falta de documentación.
- Conservar advertencias como errores. HTML/XML juntos permiten comprobar @return
  con la versión instalada; las etiquetas automáticas son inglesas y el contenido español.

Validación B04: 17 funciones/USW en XML (6 Procesamiento, 6 Comunicacion,
2 Medicion_Velocidad, 2 Mandos y main), sin avisos. Nuevas USW.2.1.5.1/.2;
contratos de Mandos incluyen escala, redondeo, errores, recuperación y concurrencia.
Límite manual común y aliases públicos también documentados.
La sonda sin @return documentada en B01 sigue como evidencia del control estricto.
El modo sólo XML instalado generó un aviso falso, reproducido incluso apareciendo
el retorno en el XML. No se suprimió la comprobación de retornos.

Referencia: [bloques documentales](https://www.doxygen.nl/manual/docblocks.html) y
[configuración oficial](https://www.doxygen.nl/manual/config.html).

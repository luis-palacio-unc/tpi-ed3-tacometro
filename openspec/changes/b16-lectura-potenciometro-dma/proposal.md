# B16 — Consultar el valor del potenciómetro

Estado: alcance corregido acordado el 2026-11-10; implementado/verificado, resultado pendiente de aceptación.

## Objetivo y origen

R03/R08/R10/R11/R15, ESW.2.1.12. El ADC convierte la entrada, el DMA transfiere
el dato a memoria y Potenciometro ofrece únicamente el valor leído.
Se retiran las validaciones/estados añadidos en la propuesta anterior.

## Alcance

- uint16_t Pot_Read(void): devuelve el valor numérico actual del potenciómetro
  almacenado por ADC/DMA, sin parámetros ni estructura de estado.
- ADC_DMA_Pot se define una sola vez en Potenciometro.c, propietario de su
  lectura. El encabezado interno Adquisicion_ADC_DMA.h declara extern para que
  el soporte configure su dirección como destino DMA al integrar.
- Si la palabra transferida incluye otros bits, extraer únicamente RESULT con
  la definición original del driver. No comprobar DONE, OVERRUN o etiquetas.
- No crear ADC_Decode, ADC_Estado, callbacks, contextos ni validadores.
- Documentar USW.2.1.12.1 con Doxygen. AD0.0/P0.23 función01 acordado para integración.

Archivos: Potenciometro.h/.c y enlace interno en soporte
ADC/DMA existente; pruebas host, CMake/Doxyfile y documentación. No modificar
Drivers, servicios, LM35, Motor o contratos/archivos aceptados.

## Responsabilidades y límites

ADC: conversión. DMA: transferencia a memoria. Soporte ADC/DMA: configuración
compartida. Potenciometro: poseer y devolver su lectura. Mandos:
normalizar, conservando su contrato. No pasar una palabra DMA desde el llamador.

Configuración de ADC/DMA/pines/reloj, asociación canal-destino y conexión productiva
se resolverán al integrar. B16 reserva una palabra de32 bits en .bss local accesible
por DMA con MCU activo, sin modificar el linker. Revisar ubicación si se usa Sleep.
Pot_Read no inicia conversiones
o transferencias, no espera ni interpreta disponibilidad/fallos; tampoco normaliza.

## Criterio de aceptación

1. Contenido0/2048/4095 en la posición del potenciómetro produce esos retornos.
2. Al cambiar el contenido, la próxima llamada refleja el valor nuevo; repetir
   lecturas no cambia la memoria ni inicia operaciones de periféricos.
3. Variable definida en Potenciometro.c, extern interno y test enlazado sin
   Adquisicion_ADC_DMA.c. Sin estados/validación ni cambios de biblioteca/otros módulos.
4. Pruebas host, compilación ARM, regresiones y Doxygen correspondientes pasan.

Formato acordado: palabra ADGDR en ADC_DMA_Pot; dependencia pendiente: configurar
la asociación del canal al destino al integrar. No prometer adquisición física
por probar memoria. Reversión por commit inverso sólo B16.
Alcance acordado e implementado por orden del usuario;
commit local al terminar, sin push. Aceptación del resultado/sincronización/archivo pendientes.

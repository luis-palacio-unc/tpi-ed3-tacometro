# Diseño candidato B16 — lectura de datos del potenciómetro

## Base y decisiones

Potenciometro y Adquisicion_ADC_DMA son esqueletos; Mandos B04 ya normaliza.
LM35 B05 ya convierte códigos a Celsius y no se modifica. La inicialización
está pospuesta; este bloque entrega dos operaciones mínimas sobre datos DMA
ya disponibles, no un controlador de adquisición completo.

| Elección | Razón |
|---|---|
| Pot_Read(dato_dma,muestra) | Recibe una palabra ya transferida; sin contexto/cache ni leer ADC a espaldas del DMA. |
| ADC_Decode(dato_dma,canal,muestra) compartido | Validación/extracción única para futuros lectores; biblioteca sólo ofrece getters de registros. |
| Macros ADC_GDR_* originales | No duplicar máscaras/desplazamientos de la biblioteca ni modificarla. |
| Canal0/P0.23 función01 propuesto | Manual/driver coinciden; no interfiere con pines acordados. Conexión/pinmux real pendientes. |
| ADC_Estado con cinco valores | Distingue cero válido, ausencia y fallos reales; puede adaptarse a estados genéricos de servicio al integrar. |

## Operación y errores

En soporte/Adquisicion_ADC_DMA.h definir ADC_Estado (ADC_VALIDA, ADC_SIN_DATOS,
ADC_ERROR_ARGUMENTO, ADC_ERROR_OVERRUN, ADC_ERROR_CANAL) y ADC_Decode.
Header sólo stdint, sin CMSIS. .c incluye lpc17xx_adc.h original y usa macros
ADC_GDR_RESULT/CH/OVERRUN_FLAG/DONE_FLAG, sin llamadas a getters físicos.

Validar salida NULL; si existe, limpiar a cero. Validar canal esperado0..7,
luego OVERRUN, DONE y canal del dato. Extraer resultado sólo con estado VALIDA.
No leer salida anterior, guardar última muestra o introducir inicialización.
Potenciometro.h incluye el contrato de soporte por ruta local y declara Pot_Read;
su .c delega con ADC_CHANNEL_0 original. No nueva configuración o escala genérica.

Flujo futuro: transferencia DMA terminada → copia estable de palabra → Pot_Read
→ ADC_Decode → código/estado → adaptación a Mandos. En B16 sólo están las dos
funciones centrales; captura/ensamblado/consumo productivo se dejan para el final.
Un valor por copia no garantiza origen/frescura: dueño del buffer debe validar
fin de transferencia y evitar lectura mientras DMA lo reescribe. No simular
ese ciclo con nuevos buffers globales o IRQ dentro de este bloque. DONE indica
fin de conversión ADC; no acredita fin de transferencia DMA.

## Recursos, pruebas y pendientes

Resultado uint16_t0..4095, extracción32bits sin aritmética de conversión/64bits.
Mandos usa máximo inclusivo4095; LM35 usa escala4096 cuentas por referencia,
contratos distintos que se conservan. ADC.2/secuencia/fuente DMA en exploración.
No fijar VREF/tasa de muestreo, reservar DMA/timer o escribir pinmux ahora.

Archivos: cuatro .h/.c de dispositivo/soporte, tests/host/test_adc_datos.c y
test_potenciometro.c, targets CMake y Doxyfile. TPI_SOURCES ya incluye ambos
esqueletos: conservar18 objetos. Nuevas USW.2.1.18.1 (soporte local) y .12.1;
sin comentarios dentro del cuerpo de funciones de producción.

Casos: palabras numéricas explícitas contra valor/canal/flags; NULL y canal>7,
bits reservados, prioridades combinadas y recuperación. Fixture Pot_Read→Mandos
real con0/2048/4095, sin modificarlo ni integrar main. No normalizar un resultado
distinto de VALIDA. No revalidar la biblioteca ejecutando su código de registros.
Ejecutar host/san/regresiones, ARM O0/O2/header aislado y recursos/símbolos,
Doxygen estricto y hashes. Palabras de prueba no simulan ADC/DMA, latencia,
señal eléctrica o adquisición física. Aceptar propuesta antes de implementar.

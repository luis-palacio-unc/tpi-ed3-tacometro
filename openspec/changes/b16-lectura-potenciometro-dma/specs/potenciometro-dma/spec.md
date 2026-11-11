# Potenciómetro — candidato B16 corregido

Origen R03/R08/R10/R11/R15 y aclaración del usuario. Alcance acordado; resultado aún no aceptado.

## Requirements

### Requirement: PD1 — Getter del valor almacenado

Potenciometro MUST exponer uint16_t Pot_Read(void). MUST devolver el valor numérico
del potenciómetro que ADC/DMA dejó en memoria, sin parámetros de entrada ni estado
adicional. Si el formato físico incluye otros campos, MUST usar la extracción
RESULT original para obtener el valor, sin interpretar flags o canal del dato.

#### Scenario: S1 — Lecturas de referencia
- Given almacenamiento del potenciómetro con muestras0/2048/4095.
- When se llama Pot_Read.
- Then MUST devolver exactamente la muestra correspondiente.

#### Scenario: S2 — Actualización y repetición
- Given una muestra y luego otra escrita en su posición por el productor.
- When se consulta nuevamente o se repite la lectura.
- Then MUST reflejar el contenido actual sin modificarlo ni conservar una copia anterior.

### Requirement: PD2 — Responsabilidades acotadas

ADC MUST realizar conversión y DMA la transferencia a memoria, configurados desde
el soporte de dispositivos al integrar. Pot_Read MUST limitarse a consultar su
lectura: MUST NOT recibir datos DMA desde el llamador, iniciar conversiones,
transferencias o esperas, validar lecturas, devolver estados, limpiar datos o
normalizar. ADC_Decode/ADC_Estado de la propuesta anterior MUST quedar retirados.

ADC_DMA_Pot MUST tener una única definición en Potenciometro.c. El encabezado
interno Adquisicion_ADC_DMA.h MUST declarar extern para la futura configuración
del destino DMA. El soporte MUST NOT definir el almacenamiento del potenciómetro.
El test del getter MUST enlazar con Potenciometro.c sin Adquisicion_ADC_DMA.c.

#### Scenario: S3 — Consulta sin operaciones adicionales
- Given la lectura almacenada por ADC/DMA.
- When se llama el getter, incluso repetidamente.
- Then MUST devolver sólo el valor, sin cambiar memoria/periféricos ni generar estados.
- And el almacenamiento MUST resolverse al enlazar solamente con Potenciometro.c.

### Requirement: PD3 — Documentación y evidencia

MUST documentar ESW.2.1.12/USW.2.1.12.1 con Doxygen y comprobar lectura mediante
memoria de prueba, build ARM/regresiones aplicables y biblioteca intacta.
MUST NOT cambiar servicios, drivers o contratos/archivos aceptados. Canal0/P0.23
acordado; configuración/arranque y enlace físico quedan para el final.

#### Scenario: S4 — Verificación por modalidad
- Given getter y contenido de memoria preparado para prueba.
- When se ejecutan controles y revisión del diff.
- Then MUST registrar pruebas reales, sin confundirlas con ADC/DMA físicos ni afirmar disponibilidad o adquisición física por el getter.

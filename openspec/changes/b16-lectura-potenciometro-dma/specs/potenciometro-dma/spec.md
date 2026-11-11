# Datos ADC/DMA y potenciómetro — candidato B16

Origen R03/R08/R10/R11/R15. Propuesta sin acuerdo, no contrato vigente.

## Requirements

### Requirement: PD1 — Extracción compartida del dato DMA

ADC_Decode(uint32_t dato_dma,uint8_t canal,uint16_t *muestra) MUST interpretar
una palabra ADGDR del LPC1769, usando definiciones originales del driver.
Canal esperado MUST ser0..7. Con dato disponible/sin overrun/canal correcto,
MUST devolver ADC_VALIDA y escribir resultado12bits0..4095, incluido cero.
MUST NOT releer registros ADC ni modificar entrada, configurar o ejecutar DMA.

#### Scenario: S1 — Datos válidos y campos reservados
- Given palabras completas con muestras0/2048/4095 y canal esperado coincidente.
- When se decodifican, incluidos bits reservados variables.
- Then MUST entregar valor exacto y VALIDA, ignorando sólo bits reservados.

### Requirement: PD2 — Estados, prioridades y recuperación

ADC_Estado MUST distinguir VALIDA/SIN_DATOS/ERROR_ARGUMENTO/ERROR_OVERRUN/
ERROR_CANAL. Prioridad MUST ser salida NULL, canal esperado>7, OVERRUN,
DONE ausente y canal de origen incorrecto, en ese orden.
Muestra escribible MUST quedar cero ante cualquier retorno no válido; NULL
MUST evitar acceso a salida. Nueva entrada válida MUST recuperar sin retener error.

#### Scenario: S2 — Ausencia y errores
- Given combinaciones de flags, canal incorrecto, salidaNULL o canal esperado8/255.
- When se decodifica y luego se entrega dato válido.
- Then MUST respetar prioridades, limpiar salida escribible y recuperar; cero válido no es ausencia.

### Requirement: PD3 — Lectura específica del potenciómetro

Pot_Read(uint32_t dato_dma,uint16_t *muestra) MUST delegar en ADC_Decode con
canal0, propuesto AD0.0/P0.23 función01. MUST devolver el mismo contrato de estado
y muestra cruda. MUST NOT normalizar, aplicar VREF, filtrar, emitir PWM o cambiar
Mandos/LM35. Header público MUST conservar tipos propios sin dependencias CMSIS.

#### Scenario: S3 — Canal del potenciómetro
- Given dato válido del canal0 o dato del canal1, aunque su muestra sea cero.
- When se consulta Pot_Read.
- Then MUST entregar dato crudo sólo para canal0; canal1 debe dar ERROR_CANAL con salida cero.

#### Scenario: S4 — Compatibilidad con Mandos
- Given Pot_Read válido con muestras0/2048/4095.
- When fixture llama Mandos_NormalizarManual con fondo inclusivo4095.
- Then MUST obtener0/500/1000 válidos, sin nuevo enlace productivo o cambio del servicio.

### Requirement: PD4 — Alcance, coherencia y evidencia

MUST documentar ESW12/USW12.1 y soporte local ESW18/USW18.1 con Doxygen.
Entrada MUST venir de transferencia completada/estable, sin productor concurrente;
no prometer frescura, simultaneidad de canales o precisión eléctrica.
MUST NOT modificar Drivers, iniciar/configurar ADC/DMA, crear IRQ/colas/contextos/
heap/globales mutables, integrar main o sincronizar contratos antes de aceptar.
Configuración física/secuencia ADC.2/VREF/propiedad del buffer quedan al final.

#### Scenario: S5 — Evidencia separada del hardware
- Given funciones y fixtures con palabras DMA construidas como datos de prueba.
- When se ejecutan host/san/ARM/header/Doxygen y diff/hashes.
- Then MUST registrar evidencia actual y preservar módulos/biblioteca aceptados; adquisición física pendiente.

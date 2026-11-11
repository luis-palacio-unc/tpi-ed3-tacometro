# Delta B18/I1 — potenciometro-dma (soporte de adquisición ADC/DMA)

Candidato de la vía rápida B18. PD1 y PD3 sin cambios (Pot_Read intacto). El
soporte también alimenta la muestra del LM35 (contrato temperatura TE3).

## MODIFIED Requirements

### Requirement: PD2 — Responsabilidades acotadas
ADC MUST realizar la conversión y DMA la transferencia a memoria, configurados por
Adquisicion_ADC_DMA_Init. Pot_Read MUST limitarse a consultar su lectura, sin
iniciar conversiones, esperar, validar, devolver estados ni normalizar.
ADC_DMA_Pot MUST tener una única definición en Potenciometro.c y recibir la palabra
AD0DR0 (RESULT en bits 15:4); el soporte MUST NOT definir ese almacenamiento. El
test del getter MUST enlazar con Potenciometro.c sin Adquisicion_ADC_DMA.c.
(Antes: configuración futura, palabra ADGDR.)

#### Scenario: S3 — Consulta sin operaciones adicionales
- Given la lectura almacenada por ADC/DMA.
- When se llama el getter, incluso repetidamente.
- Then MUST devolver sólo el valor, sin cambiar memoria/periféricos ni generar estados.

## ADDED Requirements

### Requirement: PD4 — Adquisición continua de dos canales
Soporte MUST exponer bool Adquisicion_ADC_DMA_Init(void). MUST, con la biblioteca
original y en orden: deshabilitar ADC_IRQn y DMA_IRQn; ADC_Init(1500) — CLKDIV = 255,
reloj de ADC ≤ 13 MHz —; ADC_PinConfig y ADC_ChannelEnable de AD0.0/P0.23 y
AD0.1/P0.24 (función 01, sin pull-up ni pull-down); ADC_IntDisable(GLOBAL) y
ADC_IntEnable de canales 0 y 1; GPDMA_Init; armar una lista circular de dos
descriptores de 1 palabra de 32 bits, ráfaga 1, sin incrementos, AD0DR0 →
ADC_DMA_Pot y AD0DR1 → ADC_DMA_LM35, con interrupción de fin de cuenta sólo en el
segundo; GPDMA_SetupChannel canal 0 periférico→memoria con pedido del ADC; cargar
el primer descriptor en DMACC0SrcAddr/DMACC0Control; DMA_IRQn prioridad 3
habilitada; GPDMA_ChannelStart; ADC_BurstEnable al final. Ese es el único acceso
directo a registros admitido y MUST justificarse en Doxygen. MUST leer siempre
AD0DRn y nunca AD0GDR. Si GPDMA_SetupChannel falla MUST devolver false sin arrancar
canal, ráfaga ni interrupción. La interrupción del ADC MUST permanecer deshabilitada
en el NVIC.

#### Scenario: S5 — Secuencia y descriptores
- Given dobles de la biblioteca/NVIC y registros de ADC/canal 0 redirigidos a RAM.
- When se llama Adquisicion_ADC_DMA_Init.
- Then MUST observarse el orden indicado, controles 0x00480001 y 0x80480001, y el
  canal con origen AD0DR0 y control sin interrupción al arrancar.

#### Scenario: S6 — Canal ocupado
- Given GPDMA_SetupChannel devuelve ERROR.
- When se inicializa.
- Then MUST devolver false, sin ChannelStart, BurstEnable ni DMA_IRQn habilitada.

### Requirement: PD5 — Dato disponible
DMA_IRQHandler MUST, sólo si el canal 0 tiene activa la interrupción de fin de
cuenta, limpiarla, incrementar un contador de pares módulo 2^32 y marcar dato
disponible. bool Adquisicion_ADC_DMA_HayDatos(void) MUST devolver false desde Init
hasta el primer par completo y true después.
uint32_t Adquisicion_ADC_DMA_ContarPares(void) MUST devolver el contador. MUST NOT
esperar ni modificar las muestras.

#### Scenario: S7 — Primer par y frescura
- Given adquisición inicializada.
- When ocurre una entrada al ISR sin fin de cuenta y luego varias con fin de cuenta.
- Then HayDatos MUST pasar de false a true en la primera válida y el contador MUST
  avanzar de a uno, dando la vuelta sin desactivar HayDatos.

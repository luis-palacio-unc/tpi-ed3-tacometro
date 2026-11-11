# Aclaración B16 — responsabilidades

El usuario descarta los estados y la validación añadidos en la propuesta inicial.
Pot_Read debe recuperar desde memoria el valor transferido por DMA, no recibir
una palabra desde el llamador y analizarla. API acordada uint16_t Pot_Read(void).

ADC realiza la conversión; DMA realiza la transferencia solicitada por el
periférico. El soporte ADC/DMA existente conserva almacenamiento/configuración.
Potenciometro sólo consulta la posición de su lectura. Reutilizar las definiciones
originales para extraer el resultado si el formato transferido lo requiere.

Canal0/P0.23 función01 acordado según UM10360 tabla81/p118 y ADC_PinConfig.
Formato de RESULT en tabla533/p589. Referencias de configuración futura del
soporte (incluida ADC.2) no se convierten en validadores dentro de Pot_Read.
No se fija aquí VREF/tasa/arranque/buffer activo ni se modifica biblioteca.
El usuario acordó e indicó implementar B16 el 2026-11-10. Evidencia de ejecución
actual en verify-report.md; inicialización/enlace físicos siguen diferidos.

Memoria: UM10360 rev4.1 §1.9/p12 permite acceso CPU y DMA a SRAM local en activo;
§4.8.1 restringe acceso a SRAM principal durante Sleep. B16 usa .bss local existente
(4 bytes, alineación4), sin cambiar linker/arranque. Integración revisará consumo,
relojes y secuencia según ADC.2 antes de iniciar transferencias reales.

Fuente futura de configuración: GPDMA_LUTPerAddr del driver original dirige ADC a
ADGDR; ADC_GlobalGetData/ADC_ChannelGetData leen registros, no esta memoria.
[NXP errata ADC.2, rev10.6 §3.2/p7](https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf)
condiciona la secuencia con ADGDR. Resolverla al integrar, sin validadores en el getter.

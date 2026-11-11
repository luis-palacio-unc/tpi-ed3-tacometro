# B16 — Lectura del potenciómetro desde datos ADC/DMA

Estado: propuesta pendiente de acuerdo; sin implementación ni pruebas B16.

## Objetivo y origen

R03/R08/R10/R11/R15: completar la lectura cruda de Potenciometro (ESW.2.1.12)
sin adelantar inicialización. Añadir sólo la extracción/validación compartida
necesaria en soporte/Adquisicion_ADC_DMA, reutilizable por LM35 después.

## Alcance y contratos propuestos

- AD0.0/P0.23 función01 para potenciómetro, propuesto según manual/driver.
- ADC_Decode(uint32_t dato_dma,uint8_t canal,uint16_t *muestra): interpretar una
  palabra ADGDR de un bloque DMA ya completado y estable. Reutilizar macros
  ADC_GDR_RESULT/CH y flags originales, sin leer de nuevo el periférico.
- Pot_Read(uint32_t dato_dma,uint16_t *muestra) delega con canal0. Salida cruda
  0..4095; no normaliza, convierte a tensión/temperatura ni acciona PWM.
- Retorno ADC_Estado compartido: VALIDA, SIN_DATOS, ERROR_ARGUMENTO,
  ERROR_OVERRUN, ERROR_CANAL. Sólo VALIDA permite consumir, incluido cero.
  Salida escribible inválida queda cero, siguiente dato válido recupera.
- Doxygen USW.2.1.12.1 y USW.2.1.18.1 (soporte local), sin contextos/estado global.

Archivos: Potenciometro.h/.c, soporte/Adquisicion_ADC_DMA.h/.c, pruebas host,
CMakeLists.txt, Doxyfile y seguimiento. No tocar Drivers, LM35, Mandos, Motor,
aplicación/main, contratos aceptados o archivos archivados.

Excluye init/pinmux/reloj, arrancar ADC/DMA, IRQ/buffers activos, filtrado/promedios,
VREF/calibración, normalización nueva o integración productiva. Quedan al final.

## Dependencias y aceptación

Entrada por valor exige origen DMA completo, estable y validado por su dueño;
no demuestra frescura ni compatibilidad eléctrica. Fuente ADGDR/errata ADC.2 y
secuencia de adquisición se resuelven al integrar, según exploración.

1. Extraer0/2048/4095, identificar canal y conservar dato de entrada.
2. Detectar SIN_DATOS/overrun/canal incorrecto/NULL/canal esperado>7; cero de
   salida ante error escribible, prioridades definidas y recuperación.
3. Fixture Pot_Read→Mandos con fondo4095 produce0/500/1000, sin modificar servicio.
4. Host/san/regresiones/ARM/header/Doxygen pasan; bibliotecas intactas.

Riesgo de buffer activo: su dueño debe garantizar estabilidad al integrar.
Pruebas de flags/canales distinguen error de cero válido; no detectan DMA concurrente.
Reversión: commit inverso sólo B16. Implementar tras acuerdo, commit local sin push.

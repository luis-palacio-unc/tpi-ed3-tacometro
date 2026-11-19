# Temperatura — contrato vigente

Origen R07/R08/R09/R10/R11/R15. B05 corregido aceptado el 2026-11-07;
revisión B05-R1 aceptada por el usuario el 2026-11-11 (implementación 94d3134).
TE1/TE3/TE4 sincronizados; TE2/S4/S5 conservados sin cambios.
Alcance lógico; adquisición y aceptación física pendientes.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
funcionamiento en placa aceptado por el usuario el 2026-11-19 (prueba funcional
del sistema completo; ver `docs/agente/ESTADO.md`).
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

Cambio de sensor (2026-11-16): el LM35 del anteproyecto no se consiguió y se
reemplazó por un LM335 en el mismo canal AD0.1/P0.24. El dispositivo pasó a
llamarse `LM335` (`Dispositivos/LM335.h/.c`, `getTemperatura`, `ADC_DMA_LM335`) y su
conversión sigue TE6. Los valores de S2, S6 y S11 se recalcularon para ese modelo;
los informes archivados conservan los nombres y valores del LM35.

## Requirements

### Requirement: TE1 — Conversión específica en dispositivo
Sin cambios salvo: ADC_VREF_MV MUST conservar #ifndef/#error y su default MUST ser
3300 mV, correspondiente a VREFP alimentado a 3,3 V en la placa; el valor físico
MUST medirse en la puesta en marcha. (Antes: default provisional 2540 mV.)

#### Scenario: S2 — Overrides y configuración inválida
- Given target host sin override y probes con valores no positivos.
- When se consulta la muestra 1024.
- Then el default MUST producir -19065 centésimas (82,50 K nominales menos 273,15);
  los probes inválidos MUST fallar por #error ADC_VREF_MV. (Antes, con LM35: 8250.)

### Requirement: TE3 — Aislamiento y responsabilidades de dispositivo

LM335 MUST poseer una única definición de muestra en LM335.c, enlazar el test sólo
con LM335.c (sin Adquisicion_ADC_DMA.c), convertir al consultar y
MUST NOT exponer enum/registro de estados, inicializador, salida por puntero o
disponibilidad. MUST NOT validar flags/canal/rango por llamada, limpiar muestra,
guardar temperatura obsoleta, esperar o leer ADC. Configuración válida, primer dato
y asociación correcta canal-destino MUST garantizarse al integrar. Cero inicial
MUST NOT interpretarse como señal de disponibilidad.

Temperatura MUST conservar TE2 independiente de LM335/ADC/VREF. LM335 MUST NOT
depender del servicio. El fixture MUST entregar las centésimas sin reescalar ni
redondear otra vez; estados genéricos se suministran externamente según el escenario.

#### Scenario: S6 — Actualización con campos ajenos
- Given palabras0xC700200F→0xC300800F y luego0x00008000, con VREF2000mV.
- When se consulta/repite tras cada escritura del productor de prueba.
- Then MUST obtener -24815→-17315→-17315 (25,00 K y 100,00 K nominales), sin mutar palabras, caché o rechazo por DONE/OVERRUN/CHN/bits3:0.

#### Scenario: S7 — Compatibilidad sin conversión de unidades
- Given centésimas del wrapper y otra fuente genérica del servicio.
- When el fixture ejecuta Temperatura_Actualizar con cada origen.
- Then MUST conservar exactamente las centésimas y el contrato de otra fuente; servicio sin parámetros físicos.

### Requirement: TE4 — Evidencia y alcance

B05-R1 MUST documentar ESW/USW y pasar host/san, ARM, configuración negativa y
Doxygen. MUST actualizar fixtures sin suprimir controles y preservar Drivers.
MUST NOT configurar/iniciar ADC/DMA, integrar main/alarma/regulación ni afirmar
diagnóstico, frescura o aceptación física por pruebas RAM. Evidencia previa o de
Claude MUST distinguirse de pruebas nuevas ejecutadas sobre esta revisión.

#### Scenario: S8 — Verificación por modalidad
- Given API/fixtures revisados, referencias explícitas y escenarios S1/S2/S3/S6/S7.
- When se ejecutan controles, revisión de diff y hashes.
- Then MUST registrar resultados reales por modalidad, dejando adquisición física pendiente.

### Requirement: TE5 — Lectura filtrada
`Servicios/Temperatura` MUST exponer `int32_t Temperatura_Leer(void)`: toma
`getTemperatura()` y devuelve el promedio entero (truncado) de las últimas 4 lecturas,
en centésimas de °C; mientras haya menos de 4 MUST promediar las disponibles.

#### Scenario: S9 — Arranque del filtro (`i2.temperatura` S1)
- Given lecturas 2000, 3000, 4000 y 5000 desde el arranque.
- When se lee cuatro veces.
- Then MUST dar 2000, 2500, 3000 y 3500.

#### Scenario: S10 — Ventana de cuatro (`i2.temperatura` S2)
- Given la ventana anterior y luego lecturas constantes de 6000 y de 2501.
- When se lee.
- Then MUST dar 4500, 5250, 5750, 6000, 6000 y luego 5125, 4250, 3375, 2501.

#### Scenario: S11 — Servicio sobre el LM335 real (`i2.temperatura_lm335`, `b05.lm335_enlace_arm`)
- Given `LM335.c` real con ADC_VREF_MV = 3300 y palabras de ADC con los códigos 3701
  (25,03 °C) y 3949 (45,01 °C).
- When se lee por el servicio.
- Then MUST dar 2503, 3502, 3835, 4001 y 4501, sin modificar la muestra.

### Requirement: TE6 — Modelo del LM335
`getTemperatura` MUST convertir con el modelo nominal de 10 mV/K: centésimas de kelvin =
código × ADC_VREF_MV × 10 / 4096 con redondeo al cercano y empate arriba; resultado
= ese valor − 27315 + `LM335_AJUSTE_CENTESIMAS`, en `int32_t` con signo. El ajuste
MUST definirse en `Configuracion/Analogica.h` con #ifndef, default 0 y #error fuera
de ±5000. El dispositivo MUST NOT saturar ni validar el rango: 0 V se informa como
−273,15 °C y el fondo de escala como 56,77 °C con 3300 mV.

#### Scenario: S12 — Kelvin a Celsius y ajuste (`b05.lm335`, `b05r1.lm335_ajuste_*`, `b05r1.oraculo_lm335`)
- Given ADC_VREF_MV = 3300, ajuste 0 y los códigos 0, 3390, 3701, 3949 y 4095.
- When se consulta.
- Then MUST dar −27315, −3, 2503, 4501 y 5677; con ajuste −150 o 275 los 4096
  códigos MUST coincidir con el oráculo racional desplazado en ese valor.

#### Scenario: S13 — Ajuste inválido (`b05r1.configuracion_lm335`)
- Given probes con `LM335_AJUSTE_CENTESIMAS` de −5001, 5001 y 27315.
- When se compila `Analogica.h`.
- Then MUST fallar por #error del ajuste; −5000, 0, 150 y 5000 MUST compilar.

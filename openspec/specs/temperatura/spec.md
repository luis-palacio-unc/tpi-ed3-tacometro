# Temperatura — contrato vigente

Origen R07/R08/R09/R10/R11/R15. B05 corregido aceptado el 2026-11-07;
revisión B05-R1 aceptada por el usuario el 2026-11-11 (implementación 94d3134).
TE1/TE3/TE4 sincronizados; TE2/S4/S5 conservados sin cambios.
Alcance lógico; adquisición y aceptación física pendientes.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: TE1 — Conversión específica en dispositivo
Sin cambios salvo: ADC_VREF_MV MUST conservar #ifndef/#error y su default MUST ser
3300 mV, correspondiente a VREFP alimentado a 3,3 V en la placa; el valor físico
MUST medirse en la puesta en marcha. (Antes: default provisional 2540 mV.)

#### Scenario: S2 — Overrides y configuración inválida
- Given target host sin override y probes con valores no positivos.
- When se consulta la muestra 1024.
- Then el default MUST producir 8250 centésimas; los probes inválidos MUST fallar
  por #error ADC_VREF_MV. (Antes: 6350.)

### Requirement: TE3 — Aislamiento y responsabilidades de dispositivo

LM35 MUST poseer una única definición de muestra en LM35.c, enlazar el test sólo
con LM35.c (sin Adquisicion_ADC_DMA.c), convertir al consultar y
MUST NOT exponer enum/registro de estados, inicializador, salida por puntero o
disponibilidad. MUST NOT validar flags/canal/rango por llamada, limpiar muestra,
guardar temperatura obsoleta, esperar o leer ADC. Configuración válida, primer dato
y asociación correcta canal-destino MUST garantizarse al integrar. Cero inicial
MUST NOT interpretarse como señal de disponibilidad.

Temperatura MUST conservar TE2 independiente de LM35/ADC/VREF. LM35 MUST NOT
depender del servicio. El fixture MUST entregar las centésimas sin reescalar ni
redondear otra vez; estados genéricos se suministran externamente según el escenario.

#### Scenario: S6 — Actualización con campos ajenos
- Given palabras0xC700200F→0xC300800F y luego0x00008000, con VREF2000mV.
- When se consulta/repite tras cada escritura del productor de prueba.
- Then MUST obtener2500→10000→10000, sin mutar palabras, caché o rechazo por DONE/OVERRUN/CHN/bits3:0.

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
`LM35_Read()` y devuelve el promedio entero (truncado) de las últimas 4 lecturas,
en centésimas de °C; mientras haya menos de 4 MUST promediar las disponibles.

#### Scenario: S9 — Arranque del filtro (`i2.temperatura` S1)
- Given lecturas 2000, 3000, 4000 y 5000 desde el arranque.
- When se lee cuatro veces.
- Then MUST dar 2000, 2500, 3000 y 3500.

#### Scenario: S10 — Ventana de cuatro (`i2.temperatura` S2)
- Given la ventana anterior y luego lecturas constantes de 6000 y de 2501.
- When se lee.
- Then MUST dar 4500, 5250, 5750, 6000, 6000 y luego 5125, 4250, 3375, 2501.

#### Scenario: S11 — Servicio sobre el LM35 real (`i2.temperatura_lm35`, `b05.lm35_enlace_arm`)
- Given `LM35.c` real con ADC_VREF_MV = 2000 y palabras de ADC de 25,00 y 100,00 °C.
- When se lee por el servicio.
- Then MUST dar 2500, 6250, 7500, 8125 y 10000, sin modificar la muestra.

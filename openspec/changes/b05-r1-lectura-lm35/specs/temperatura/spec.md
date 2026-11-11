# Delta B05-R1 — temperatura / dispositivo LM35

Candidato corregido por revisión/decisiones del usuario. TE2/S4/S5 sin cambios;
sólo sincronizar tras aceptar el resultado. H5 queda sin efecto: no hay float.

## MODIFIED Requirements

### Requirement: TE1 — Conversión específica en dispositivo

LM35 MUST exponer int32_t LM35_Read(void), centésimas Celsius no negativas desde
su muestra ADC/DMA;2500 significa25,00°C. MUST usar aritmética entera, ganancia
nominal10mV/°C y4096 cuentas. MUST extraer sólo RESULT original, sin interpretar
DONE/OVERRUN/CHN/bits3:0. MUST redondear una sola vez al cercano, empate arriba,
con error aritmético≤0,005°C respecto del modelo; sin float/double, recorte o
redondeo intermedio. Valor ideal en centésimas=muestra×ADC_VREF_MV×10/4096.
(Antes: muestra/escala/referencia por llamada, salida por puntero y estados.)

ADC_VREF_MV MUST ser configurable con #ifndef/default2540mV y #error si no es
positivo o excede INT32_MAX/10, para representar el resultado. Default provisional
de software MUST NOT atribuirse a la placa. VREF física MUST verificarse al integrar.

#### Scenario: S1 — Referencias y extracción exclusiva
- Given VREF de prueba2000mV, palabras para muestras0/512/2048, limpias y OR0xC700000F.
- When se consulta LM35_Read.
- Then MUST devolver0/2500/10000 centésimas en ambas variantes, preservando la palabra.

#### Scenario: S2 — Overrides y configuración inválida
- Given targets host separados con LM35.c/test compilados mediante target_compile_definitions PRIVATE ADC_VREF_MV=1000 o2000, sin afectar firmware; otro sin override.
- When se consulta muestra1024 y se compilan probes C11 con -DADC_VREF_MV=0 y -DADC_VREF_MV=-1.
- Then targets MUST devolver2500/5000; default MUST producir6350 y cada probe no positivo MUST fallar por #error ADC_VREF_MV.

#### Scenario: S3 — Dominio completo y redondeo B05
- Given4096 códigos por build con overrides1000/2000/2540/3300/5000mV.
- When se compara con floor(Fraction(muestra×VREF_mV×10,4096)+Fraction(1,2)), usando racionales exactos independientes.
- Then MUST coincidir exactamente en todos los códigos; VREF1000/muestra128 MUST dar313 (empate), sin overflow ni error>0,005°C.

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
(Antes: enum/registro/inicializador y prioridades de errores del dispositivo.)

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

# Delta B18/I1 — temperatura (sólo referencia del ADC)

Candidato de la vía rápida B18. I1 cambia únicamente el valor por defecto de
ADC_VREF_MV por decisión del usuario (O2); el resto de TE1–TE4 no cambia en I1.
I2 puede ampliar este delta.

## MODIFIED Requirements

### Requirement: TE1 — Conversión específica en dispositivo
Sin cambios salvo: ADC_VREF_MV MUST conservar #ifndef/#error y su default MUST ser
3300 mV, correspondiente a VREFP alimentado a 3,3 V en la placa; el valor físico
MUST medirse en la puesta en marcha. (Antes: default provisional 2540 mV.)

#### Scenario: S2 — Overrides y configuración inválida
- Given target host sin override y probes con valores no positivos.
- When se consulta la muestra 1024.
- Then el default MUST producir 8250 centésimas; los probes inválidos MUST fallar
  por #error ADC_VREF_MV. (Antes: 6350.)

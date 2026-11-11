# B17 — Regulador PI de velocidad en automático

Propuesta aprobada por Claude r2, 2026-11-11, base9352f0d.
Resultado8f1793f aceptado el 2026-11-11; reauditoría de implementación r2 APROBAR, sin hallazgos nuevos.
Cierre, sincronización, archivo, commit local y push autorizados por el usuario.

## Objetivo y origen

R04 (anteproyecto §1), R06/R10/R11/R14/R15; ESW.2.1.2.
B01 selecciona fuente/consigna, pero aún no calcula mando automático.
Agregar PI entero en Procesamiento que reciba consigna 0..185 RPM y medición B03,
y produzca mando 0..1000 para Driver_Motor, mediante API separada.
Fuente externa local y decisiones resumidas en [review-notes.md](review-notes.md).

## Alcance y enfoque

- Inicializar, regular y reiniciar una instancia independiente de B01.
- Periodo/ventana contractual500ms, resolución 2 RPM con 60 pulsos/vuelta. Ganancias num/den
  ya discretizadas; Control.h con defaults provisionales y guards de compilación.
- Saturación, anti-windup, cero explícito y prioridad de errores; productos int64_t.
- Reutilizar última medición válida hasta N periodos inválidos, default 4,
  congelando integrador; exceder N o no tener historia implica cero/reinicio.
- Doxygen USW.2.1.2.8 en adelante; oráculo Fraction por paso, planta simulada
  entera con retardo, fixture B01+PI+Driver_Motor y enlace ARM sin flotantes.

## Exclusiones y dependencias

Sin main productivo, ISR, Timer, PWM, adquisición, sintonía real, heap o globals mutables.
Drivers, servicios y contratos aceptados intactos. B01 conserva tipos, funciones,
prioridades y descarte de consignas. Inicialización/enlace físico al final.
Dependencias aceptadas: B01, B03-R1, B08; Limites.h y escala del servicio B03.

## Archivos previstos al implementar

| Archivos | Cambio |
|---|---|
| Aplicacion/Procesamiento.h/.c |Agregar API/tipos separados; conservar B01/B06 |
| Configuracion/Control.h |Crear configuración lógica |
| tests/host/test_regulador.c, consulta_regulador.c |Unidad y emisor de trazas |
| tests/host/test_integracion_regulador.c |Composición sin dispositivos |
| tests/host/verificar_regulador.py, verificar_configuracion_control.py |Oráculo/planta y probes |
| cmake/verificar_enlace_b17.cmake, CMakeLists.txt |Targets/fixture nuevos |
| docs/doxygen/Doxyfile |Incluir Control.h en documentación |
| docs/agente/, README.md, openspec/ |Seguimiento, evidencia y documentación |

## Aceptación y riesgos

Aceptar sólo tras escenarios RV1–RV6/S1–S10, pruebas previstas completas,
oráculo exacto, anti-windup sensible a mutación y regresiones intactas.
Planta S8 conservada; sobrepico≤10% del escalón, establecimiento≤20 pasos a ±2 RPM confirmado;
modelo/ganancias/medición definidos en [verify-report.md](verify-report.md), sin validez física.
Riesgos: cuantización, datos viejos, overflow y reinicio omitido; mitigaciones:
ventana 500 ms, retención acotada, cotas demostradas y fixture de transiciones.

## Reversión y siguiente paso

Revertir sólo el futuro commit B17 y sus targets nuevos, preservando B01/B06.
B17 cerrado y archivado; integración a cargo de Claude. Codex no inicia otro bloque. Ver archive-report.md.

# B17 — Revisión externa local y decisiones

Fuente leída completa: `plan-B17-regulador.md`, plan externo local de Claude del
2026-11-11, incluida «Decisiones del usuario». No distribuido con el repositorio;
este resumen portable conserva lo aplicable sin enlazar rutas externas ni publicar
harness/scripts/logs. Base del plan94d3134; base de esta propuesta9352f0d tras cierre B05-R1.
Revisión posterior leída completa: `B17-propuesta.md`, incluido el Addendum final;
también revisión externa local, sin distribuir ni enlazar al harness. Veredicto
final: APROBAR CON CAMBIOS MENORES (H4, H5); r2 aprobada; implementación ahora pendiente de auditoría.

## Decisiones de usuario vigentes

| ID | Decisión |
|---|---|
| D1 |PI en Procesamiento, R04/ESW.2.1.2 |
| D2 |Control=ventana500ms,60pulsos/vuelta→2RPM por pulso; macro TPI_CONTROL_PERIODO_MS, sin Timer |
| D3 |Ganancias fracciones enteras num/den ya discretizadas para500ms, sin float |
| D4 |No VALIDA reutiliza última válida hasta N consecutivos, default provisional4; integrador congelado. Sin historia o al superar N:0/reinicio; Reiniciar descarta retenida |
| D5 |Inicializar/regular/reiniciar separados de Procesamiento_Actualizar; fixture B01+regulador+Driver_Motor, B01 intacto |
| D6 |Sobrepico≤10%; establecimiento≤20 pasos a±2RPM confirmado para el modelo, no sintonía real |

La orden actual confirma los agregados de D4 que el plan aún rotulaba pendientes.
Resuelve también el descarte de historia al reiniciar. El usuario confirmó después
el establecimiento≤20 pasos a±2RPM; es aceptación del criterio simulado, no prueba física.

## Elaboraciones de diseño para revisar

- PI posicional: salida con integral previa; luego integrar sólo con medición válida
  y sin empujar saturación. Integral limitada0..1000. Redondeo final cercano/empate arriba.
- Defaults Kp4/1000, Ki1/1000 para prueba; componentes1..1000, N1..65535.
  Configuración por macros y copia por instancia, con validación equivalente.
  Periodo500 es constante de contrato; #ifndef/#error sólo protege ese valor. Cambiarlo exige recalcular ganancias y revisar contrato.
- Estados, NULL→configuración→consigna→medición y cero/reinicio definidos en diseño.
  Todo estado no VALIDA, incluso desconocido, usa D4; NULL es error de argumento.
- Modelo con ganancia200RPM al100%, polo3/4 y retardo1paso; medición cuantizada2RPM.
  Sobrepico medido sobre amplitud del escalón; establecimiento±2RPM hasta fin de
  horizonte100pasos por escalón. Detalles en verify-report, pendientes de aceptación.

## Compatibilidad y evidencia

B01-R5 excluye PI/PID del bloque B01 y MM4 excluye regulador de B08. D1/D5 actuales
proponen nueva API/unidades en Procesamiento, manteniendo aquellas funciones/contratos:
no integrar PI en Procesamiento_Actualizar ni cambiar los fixtures aceptados.
Esta delimitación se deja explícita para revisión; no se editan specs vigentes.

Los resultados29/29host/san,13/13ARM/Doxygen del plan son evidencia externa anterior.
El cierre B05-R1 tiene evidencia propia histórica independiente. No son pruebas B17.
Todavía no existe implementación, simulación contra C ni verificación B17 aprobada.
Siguiente paso: /auditar B17; commit de implementación autorizado, sin push.


## Revisión B17 y Addendum final: decisiones aplicadas

| Hallazgo | Resolución del usuario |
|---|---|
| H1 |Retirado: conservar denominadores1..1000 y cotas originales |
| H2 |Retirado: conservar única planta de S8, sin añadir otras |
| H3 |Retirado: detección de sensor desconectado fuera de alcance |
| H6 |Retirado al final del Addendum: S8 conserva200RPM con mando1000, no185 |
| H4 |Aclarar constante contractual TPI_CONTROL_PERIODO_MS=500; cambiar exige recalcular ganancias/revisar contrato; no Timer |
| H5 |Planear mutante de redondeo por truncado, sólo copia externa; S1 u oráculo S9 deben fallar |

Dato aportado por usuario: motor físico no supera aproximadamente185RPM con PWM
al100%, medido en un trabajo anterior. Se registra en CONTEXTO como observación
previa, sin presentarlo como medición nueva, calibración ni motivo para cambiar S8.
185RPM sigue siendo consigna máxima; no recorta el dominio válido de B03.
Claude verificó cálculos/modelos en copias propias; son resultados externos, no
pruebas de firmware B17. Las únicas correcciones de diseño activas son H4/H5.
No convertir las sugerencias retiradas del cuerpo de la revisión en nuevos requisitos.


## Aprobación r2 e implementación

Fuente externa local leída completa: B17-propuesta-r2.md, veredicto APROBAR.
No se distribuye ni enlaza el harness. H4/H5 resueltos; m1 incorporado al
Given/When/Then de S1 y ejecutado en PasoYEmpate/oráculo/mutación.
Usuario ordena tareas1.2–4.6 y commit antes de /auditar B17; esa orden prevalece
sobre la pregunta externa acerca de esperar a la auditoría para commitear.
Evidencia propia en build/verificacion-b17-2026-11-11 dentro del workspace,
ignorada; reproducir con python3 scripts/verificar_b17.py.
Los resultados externos del revisor se distinguen de las pruebas propias del informe.


## Auditoría de implementación y corrección H1/H2

Fuente externa local leída completa:B17-implementacion.md,sobre9352f0d..bf975f3;
veredicto APROBAR CON CAMBIOS,sin bloqueantes. No se distribuye ni enlaza el harness.
Claude consideró correcto el código;detectó huecos en límites de integral (Ki>Kp)
y n=0/error negativo. Sus mutaciones/resultados son evidencia externa.

Usuario autoriza sólo pruebas:dos CONFIGS Ki>Kp,dos trazas y mutantes sin límites
superior/inferior y con n<0. Toda producción se conserva;copias de mutación
ignoradas dentro del workspace. Referencia Fraction sin cambios.
Verificación propia:unidad10/10,oráculo133758 pasos por modalidad;los tres nuevos
mutantes mueren en unidad y oráculo,copia correcta pasa. Detalles en verify-report.
Siguiente /auditar B17 (r2);commit local autorizado,sin push ni cierre.

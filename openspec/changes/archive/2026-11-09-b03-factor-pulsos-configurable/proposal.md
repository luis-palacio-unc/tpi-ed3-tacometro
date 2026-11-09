# B03-R1 — Factor de pulsos por vuelta configurable

Estado: resultado aceptado por el usuario el 2026-11-09; cierre SDD conforme al
procedimiento acordado. Sin push autorizado.

## Objetivo y autorización

R02 requiere disco de 60 ranuras; R15 exige servicios independientes del hardware.
El B03 aceptado fija 60 dentro de Medicion_Velocidad. La orden del usuario del
2026-11-07 autoriza resolver esta revisión mediante un cambio independiente.
La aclaración R15 prevalece para el nuevo contrato candidato; el archivo B03 se conserva.

## Alcance

Recibir pulsos_por_vuelta uint32_t positivo como cuarto argumento por llamada.
Mantener uint32_t pulsos/duración_us, salida en milésimas de RPM, cercano/empate
arriba, disponibilidad, recuperación y mediciones sin recorte a 185 RPM.
Factor cero invalida con error propio. El valor 60 del TPI queda en
Configuracion/Medicion.h; el servicio no incluye esa configuración ni dispositivos.
No conservar un wrapper con 60 implícito. Actualizar consumidores actuales, USW y pruebas.

Excluye captura/timer/ISR físicos, elección de ventana, engranajes, frescura,
filtrado, aplicación/main y regulación. No abrir B06 ni modificar archivos aceptados.

## Dependencias y archivos

B00–B05 aceptados; toolchain/Python/Doxygen existentes. Afecta servicio .h/.c,
Configuracion/Medicion.h, pruebas C y oráculo Python estándar, fixture ARM B03,
CMakeLists/Doxyfile y contexto SDD. No agrega dependencias instalables.

## Aceptación y riesgos

- Factor 60 conserva resultados B03; factores 1/12/100/1024 funcionan sin recompilar servicio.
- Cero, errores combinados, extremos uint32_t, empate y recuperación son verificables.
- Cálculo exacto sin overflow incluso cuando el numerador matemático excede uint64_t.
- Host/sanitizadores, oráculo de enteros arbitrarios, ARM Debug/Release y Doxygen pasan.
- Servicio compila aislado sin incluir configuración o dispositivos; evidencia por escenario.

Riesgos: ruptura explícita de API y redondeo/desbordes de fórmula general.
Mitigar migrando todos los llamadores y usando oráculo distinto del algoritmo C.
Reversión: revertir sólo el commit de revisión, conservando trabajo ajeno/historial.
Resultado requiere aceptación antes de sincronizar/archivar; commit local autorizado,
sin extender el push de B05 al cambio nuevo.

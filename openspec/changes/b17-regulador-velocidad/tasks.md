# Tareas B17 — implementación verificada

Propuesta r2 aprobada; usuario autoriza tareas1.2–4.6, m1 y commit local. Evidencia en verify-report.md.

## 1. Revisión y configuración

- [x] 1.1 Resolver `/revisar-propuesta B17` (r2) sobre proposal/spec/design/review-notes y acordar alcance; preservar HEAD/cambios locales.
- [x] 1.2 Crear Configuracion/Control.h: ganancias/N configurables, periodo constante contractual500ms con guard/@details; mantener Limites.h (H4/RV4/S5/S6).
- [x] 1.3 Agregar tipos/API independientes y Doxygen USW.2.1.2.8–10 en Aplicacion/Procesamiento.h; preservar tipos B01/B06.

## 2. PI y fronteras

- [x] 2.1 Agregar en Aplicacion/Procesamiento.c inicialización/copia validada y reinicio sin historia, preservando funciones previas (S4/S5).
- [x] 2.2 Escribir tests/host/test_regulador.c para paso/empate/saturación/errores/retención/N+1/instancias/extremos (S1–S6).
- [x] 2.3 Agregar Regular según orden/política/aritmética de design.md, sin dependencias de dispositivos (S1–S6).

## 3. Evidencia independiente y composición

- [x] 3.1 Crear tests/host/consulta_regulador.c como emisor de trazas, sin nuevas APIs productivas de inspección (S9).
- [x] 3.2 Crear tests/host/verificar_regulador.py con Fraction,8configuraciones/LCG/65536pasos y fronteras N; comparar salida y estado por paso (S9).
- [x] 3.3 Agregar al mismo verificador planta fija/200pasos, métricas por escalón y desaturación40pasos/J1 (S2/S8).
- [x] 3.4 Crear tests/host/test_integracion_regulador.c: B01+regulador+Driver_Motor, nuevas consignas/reentrada/retornos del proveedor (S7).
- [x] 3.5 Crear tests/host/verificar_configuracion_control.py:4builds positivos/21negativos por diagnóstico específico (S6).
- [x] 3.6 Añadir sólo targets/CTest nuevos en CMakeLists.txt, mismo C11 estricto y sanitizadores; preservar29suites anteriores (S10).

## 4. ARM, revisión y entrega

- [x] 4.1 Crear cmake/verificar_enlace_b17.cmake: fixture de API/composición, build Debug/Release, símbolos retenidos y ausencia de float; preservar13controles (S10).
- [x] 4.2 Incluir Control.h en docs/doxygen/Doxyfile y ejecutar host/san/ARM/Doxygen; registrar comandos/retornos/sizeof/pila/O0/O2/libgcc en verify-report.md.
- [x] 4.3 En copia externa, quitar anti-windup dejando saturación y comprobar fallo S2; nunca alterar Drivers o desactivar controles.
- [x] 4.4 En copia externa de Procesamiento.c, mutar redondeo a truncado n/d; comprobar fallo S1 u oráculo S9 y caso correcto1 frente a mutante0; guardar evidencia (H5).
- [x] 4.5 Revisar diff/hashes de Drivers, servicios, funciones/tipos previos y specs aceptadas; actualizar README/ESTADO/CONTEXTO/MAPA/config/state.
- [x] 4.6 Tras orden de implementación, entregar para auditoría según autorización de Git vigente; no inferir cierre, sincronización, archivo o push.

18/18 tareas iniciales completas. Corrección H1/H2 autorizada sólo en pruebas.


## 5. Corrección de auditoría H1/H2

- [x] 5.1 Añadir configuraciones Ki>Kp y trazas H1/H2 en tests/host/verificar_regulador.py/test_regulador.c;conservar producción.
- [x] 5.2 Ampliar scripts/verificar_b17.py con mutantes de límites superior/inferior y n=0;copia correcta pasa y mutantes fallan.
- [x] 5.3 Ejecutar verificación completa,actualizar evidencia/seguimiento,revisar diff y commit local sin push para /auditar B17 (r2).

21/21 tareas al commit de corrección;sin cierre/sincronización/archivo/push.

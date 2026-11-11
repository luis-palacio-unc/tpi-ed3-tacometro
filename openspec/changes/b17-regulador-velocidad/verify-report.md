# Verificación B17 — lista para auditoría

2026-11-11, main, base9352f0d. Propuesta Claude r2 APROBAR; usuario autoriza tareas
1.2–4.6 y commit local. m1 resuelto en Given/When/Then S1: Kp=Ki=1/1000,
consigna1RPM/medición válida500milésimas→mando1. Resultado pendiente de auditoría
y aceptación; sin cierre, sincronización, archivo o push.

## Cambios y reproducción

Tres APIs separadas de B01: InicializarRegulador/Regular/ReiniciarRegulador,
USW.2.1.2.8–10. PI entero int64_t, saturación/redondeo/anti-windup condicional;
retención N sin integrar, reinicio descarta historia. Prioridad de errores y
dominio uint32_t de medición conservados. Control.h ganancias/N;500ms contractual.

Desde raíz: `python3 scripts/verificar_b17.py`. Sin red/hardware/dependencias nuevas.
Ejecuta host, sanitizadores, ARM, Doxygen, mutaciones, recursos y preservación. Admite
`--fase host|san|arm|docs|mutations|resources|preservation` (resources requiere arm previo).
Evidencia local ignorada por Git, dentro del workspace:
`build/verificacion-b17-2026-11-11`. `--evidencia` permite otra carpeta del proyecto.
No se usa /tmp para evidencia B17; temporales de las regresiones host/san también
se redirigen al workspace. El protocolo aprobado previo se conserva allí como
approved-verification-plan.md, separado de este informe actual.

Cada fase genera *-attempt-NNN con commands.json (invocaciones/retornos/entorno),
logs, versiones, source-inputs.json y results.json; *-latest.txt identifica su
última ejecución. La ejecución integral final se identifica con esos punteros; intentos anteriores
se conservan separados para no sobrescribir evidencia.

## Pruebas propias ejecutadas

| Modalidad/control | Resultado | Evidencia dentro de esa carpeta |
|---|---|---|
| Host Debug |33/33 suites,0 omitidas:29 anteriores+4 B17 |host-attempt-003/tests.log |
| ASan/LSan/UBSan |33/33,0 omitidas,sin hallazgos; detect_leaks=1/halt_on_error=1 |san-attempt-003/tests.log,commands.json |
| Unidad PI |8/8 casos:paso/empate,AW,retención,N,recuperación,cfg,prioridades,extremos/instancias |b17.regulador en host/san |
| Composición B01+PI+B08 |2/2 casos;15 emisiones de transiciones/errores+3 de retención |b17.integracion_regulador |
| Oráculo Fraction |116722 pasos exactos por modalidad |host/san/evidencia-b17/oraculo |
| Planta entera |200 pasos con C/Fraction, dos escalones conformes |oraculo/plant-trace.json,results.json |
| Configuración |4 builds positivos/21 negativos por diagnóstico específico |host/san/evidencia-b17/configuracion |
| ARM |Build y15/15 controles:13 previos+fixture B17 Debug/Release |arm-attempt-003/commands.json,tests.log |
| Doxygen HTML/XML |46 funciones/USW,0 avisos, tres nuevas documentadas |docs-attempt-003/doxygen |
| Recursos O0/O2 |Instancia40bytes≤48, sin auxiliares float |resources-attempt-003 |
| Preservación/diff |B01/B06,Servicios,Dispositivos,Drivers,main,specs/archivo intactos |preservation.json,review.diff |

C11 estricto: -Wall/-Wextra/-Werror/-Wconversion/-Wshadow/-Wstrict-prototypes/
-pedantic-errors.224 ejecuciones de casos C por modalidad (214 anteriores+8+2),
además de oráculos/probes. Regresiones B03/LM35/selector y29 suites previas presentes.
Sin omisiones ni umbral de cobertura configurado. No se desactivaron controles.

Fraction usa I racional y verifica estado,mando,I,historia/contador por paso.
Seis configuraciones,32×256 pasos pseudoaleatorios cada una, LCG32 semilla0x0B170017.
116722=67378 deterministas+49152 pseudoaleatorios+192 inicializaciones de secuencia.
Corpus incluye N1/4/65535 hasta N+1 mediante llamadas reales, empates, consignas/
mediciones extremas y recuperación. Archivos oracle-input/expected/actual.txt y
oracle-stderr.log permiten reproducir/verificar cada fila. Sin tolerancias float.
La planta compara aparte su inicialización y200 pasos.

## Planta S8 observada

Modelo acordado:200RPM al100%, polo3/4, retardo1paso, medición cuantizada a2RPM.
No se cambió por el dato histórico del motor físico. Inicio cero,100 pasos a100RPM,
luego100 a185RPM, sin reinicio de integral.

| Escalón | Pico RPM | Exceso RPM | Establecimiento |
|---|---|---|---|
| 0→100 |99,600 |0 |14 pasos/7s |
| 100→185 |185,200 |0,200 |14 pasos/7s |

En cada tramo queda dentro de±2RPM desde ese paso hasta completar100.
10×exceso≤amplitud del escalón y j≤20 confirmados se cumplen en ambos.
Esto acredita el modelo acordado; no es sintonía ni funcionamiento físico.

## Mutaciones ejecutadas

Copias en mutations-attempt-003/{correct,anti-windup,rounding}; fuentes/headers/
tests, ejecutables, mutation.diff y trazas conservados. Las tres compilan exit0
con warnings como errores. Copia correcta pasa unidad y oráculo con exit0.

| Mutante | Unidad | Oráculo | Fallo observado |
|---|---|---|---|
| Quitar condición AW conservando clamps |Exit1,FAIL S2 |Exit1,S9 discrepancia de integral |I cambia cuando debía quedar370000 |
| Truncar n/d en vez de redondear |Exit1,FAIL S1 |Exit1,S9 paso1,esperado1/obtenido0 |Empate explícito m1 |

*-unit.log/*-oracle.log,commands.json/results.json registran los fallos esperados.
Producción correcta intacta; ninguna mutación se aplica al árbol versionado.

## Matriz de conformidad

| Requisito/escenario | Prueba ejecutada | Resultado |
|---|---|---|
| RV1/S1 |PasoYEmpate,oráculo,mutante truncado |Conforme |
| RV2/S2 |AntiWindup,40pasos/J1,mutante AW |Conforme |
| RV3/S3 |RetencionYRecuperacion,RachaInterrumpidaYConsigna,oráculo |Conforme |
| RV3/S4 |FronterasN,reinicio/sin historia/instancias |Conforme |
| RV4/S5 |Configuracion,PrioridadesYCero,recuperación |Conforme |
| RV4/S6 |ExtremosEInstancias,oráculo,25 probes |Conforme |
| RV5/S7 |Transiciones,RetencionCompuesta,retorno/fallo del proveedor |Conforme lógico |
| RV5/S8 |Modelo fijo,trazas y métricas por tramo |Conforme al modelo |
| RV6/S9 |116722 coincidencias exactas del estado completo |Conforme |
| RV6/S10 |Regresiones,ARM,Doxygen,recursos,diff/hashes |Conforme por modalidad |

## Recursos y límites

Instancia ARM40bytes, sin heap ni estado global mutable nuevo.
Tamaños de funciones Init/Regular/Reiniciar: O0=186/1382/64bytes; O2=124/472/32bytes.
Pila propia -fstack-usage: O0=48/288/16bytes, O2=16/56/8bytes.
División: __aeabi_ldivmod→__udivmoddi4; B03 también enlaza __aeabi_uldivmod.
Desensamblado de esta toolchain:16bytes del wrapper+32 del helper=48 adicionales.
Regular usa336bytes O0 o104 O2 en esa ruta; no incluye llamador/excepciones/ISR
ni integración. Logs separan objetos/símbolos/pila propia y auxiliares libgcc.
Sin __aeabi_f* o __aeabi_d* en objetos/ELF inspeccionados. Sin medición de ciclos.

10/10 escenarios conformes a su modalidad;18/18 tareas al commit. Veredicto propio:
PASS. Auditoría Claude/aceptación del usuario pendientes; sin sincronizar/archivar/push.
Sin ejecución MCU, simulador CPU, pruebas de motor, flasheo o cambios de reloj.
Cadencia500ms,main/ISR/Timer/PWM/ADC-DMA y sintonía real quedan para integración.
Sensor desconectado fuera de alcance.

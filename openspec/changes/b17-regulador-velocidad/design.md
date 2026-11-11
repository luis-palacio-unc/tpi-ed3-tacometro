# Diseño candidato B17

## Decisiones e interfaces

| Opción | Decisión y motivo |
|---|---|
| PI/PID |PI: D1, sin derivar medición cuantizada |
| Acoplar B01/API aparte |API aparte D5: preserva máquina/contexto de modos |
| Float/Q/fracción |Fracciones D3: oráculo exacto y ganancias legibles |
| Integral ilimitada/condicional |Condicional y cota física de mando: evita windup/overflow |

En Procesamiento.h agregar ConfigRegulador (uint32_t kp_num/kp_den/ki_num/ki_den/
max_periodos_sin_medicion), Regulador (int64_t integral_num, copia de configuración,
uint32_t ultima_valida/periodos_invalidos, bool configurado/tiene_medicion) y
EstadoRegulador, todos con prefijo Procesamiento_. Sin modificar tipos B01/B06.

| USW nueva | Firma candidata |
|---|---|
| USW.2.1.2.8 |`bool Procesamiento_InicializarRegulador(Procesamiento_Regulador *r, const Procesamiento_ConfigRegulador *cfg)` |
| USW.2.1.2.9 |`Procesamiento_EstadoRegulador Procesamiento_Regular(Procesamiento_Regulador *r, uint32_t consigna_rpm, const Medicion_Velocidad_Lectura *medicion, uint16_t *mando_por_mil)` |
| USW.2.1.2.10 |`bool Procesamiento_ReiniciarRegulador(Procesamiento_Regulador *r)` |

Estados (prefijo PROCESAMIENTO_REGULADOR_): VALIDO, RETENIENDO_MEDICION,
SIN_MEDICION, PARADO, ERROR_ARGUMENTO, ERROR_CONFIGURACION, ERROR_CONSIGNA.
Inicializar copia configuración válida por valor y limpia estado; error deja
instancia definida/deshabilitada y devuelve false. Reiniciar devuelve false sólo
para NULL; borra dinámica, conserva configuración/habilitación. No repara init fallida.

## Orden contractual

Regular escribe 0 en salida existente primero; prioridad:

| Condición | Retorno/efecto |
|---|---|
| r/medicion/salida NULL |ERROR_ARGUMENTO, reinicio de r existente |
| Init fallida/configurado=false |ERROR_CONFIGURACION, reinicio dinámico |
| Consigna>185 |ERROR_CONSIGNA, reinicio dinámico |
| Consigna 0 |PARADO, reinicio completo de dinámica |
| Medición VALIDA |Guardar uint32_t completo, contador 0; regular/integrar |
| Otra, con historia y contador<N |Incrementar contador; regular con historia, congelar I |
| Otra sin historia o contador=N |SIN_MEDICION/0, reinicio dinámico |

NULL mantiene prioridad incluso con consigna 0. No leer rpm_milesimas de un estado
no válido, incluidos enumerados desconocidos. Al N+1 se descarta también la historia.
Default 4 retiene pasos 500/1000/1500/2000 ms; quinto a 2500 ms pide 0.
Retener medición no significa retener mando: consigna nueva cambia P.

## Aritmética y cotas

Usar TPI_RPM_MAX, TPI_MANDO_MANUAL_MAX y MEDICION_VELOCIDAD_ESCALA_RPM existentes.
e=(int64_t)consigna×1000−(int64_t)medición; e∈[−4294967295,185000].
I=integral_num/ki_den, invariante0≤integral_num≤1000×ki_den.
P=e×kp_num/kp_den. v=P+I utiliza integral anterior.
Kp: por mil por milésima de RPM; Ki: mismas unidades por paso. Los defaults
equivalen a 4 por mil/RPM proporcional y 1 por mil/RPM por paso integral; no multiplicar
por 500 ms nuevamente. I se expresa en por mil.
Salida=redondear(clamp(v,0,1000)), cercano/empate arriba.
Sólo con dato válido, salvo (v≥1000∧e>0)∨(v≤0∧e<0):
integral_num siguiente=clamp(integral_num+e×ki_num,0,1000×ki_den).
En retención conservar exactamente integral_num.

C combinará n=(e×kp_num)×ki_den+integral_num×kp_den y d=kp_den×ki_den,
con promociones int64_t antes de multiplicar/restar. Comparar n con 0 y 1000d;
sólo dentro redondear (n+d/2)/d. No redondear P/I por separado.
Componentes1..1000 implican |e×num|≤4294967295000, d≤1000000,
|n|≤4294968295000000<INT64_MAX; suma de integración≤4294968295000 en módulo.
1000d≤1000000000. Convertir a uint16_t sólo tras limitar. Medición no se recorta.
H5: en copia externa sustituir redondeo por truncado n/d, conservando el resto.
Con Kp1/1000, error500 e integral0, debe obtener0 en vez de1; S1 u oráculo S9
deben detectar la mutación. La copia correcta debe pasar el mismo caso.

## Configuración, tiempo y recursos

Nuevo Control.h: ganancias/N configurables con #ifndef/default/#error:
TPI_CONTROL_KP_NUM=4, TPI_CONTROL_KP_DEN=1000, TPI_CONTROL_KI_NUM=1,
TPI_CONTROL_KI_DEN=1000 y TPI_CONTROL_MAX_PERIODOS_SIN_MEDICION=4.
Cuatro componentes 1..1000; N=1..65535. Runtime valida iguales cotas.
TPI_CONTROL_PERIODO_MS=500 es constante de contrato, no parámetro de sintonía.
Se conserva #ifndef/#error como protección: sólo admite500, incluso si se define
desde el compilador; no habilita otra cadencia. Su @details debe aclararlo.
Cambiar el periodo exige recalcular las ganancias discretizadas y revisar el
contrato antes de implementar. No configura Timer. Defaults de ganancias/N provisionales.
No API extra para defaults: llamador construye cfg con macros.
Un paso por ventana nueva de 500 ms; llamador garantiza cadencia/coherencia/frescura,
sin ráfagas de recuperación sobre el mismo dato VALIDA. No reloj/Timer interno.
Memoria por instancia fija, propuesta ≤48 bytes; medir sizeof ARM y pila propia O0/O2,
incluidos auxiliares libgcc por separado. Sin heap/globales mutables/esperas.
Llamadas serializadas, instancia inicializada incluso tras init fallida; punteros
vivos/no solapados, sin acceso simultáneo ISR. No detectar memoria arbitraria/corrupción.

## Composición y cambios

B03→lectura→Regular→mando→Driver_Motor. Sólo fixture B01+regulador+B08:
MANUAL reinicia/regresa mando manual; CERO reinicia/solicita cero; AUTOMATICA
regula y aplica VALIDO/RETENIENDO, solicitando cero para otros resultados.
Consumir retorno Driver_Motor, sin reintento/política nueva por fallo de actuador.
Entrada/reentrada B01 debe pasar por CERO antes de consigna nueva.

Archivos futuros y exclusiones: [proposal](proposal.md); trazas/probes/modelo y
regresiones: [verify-report](verify-report.md). Preservar funciones previas byte a
byte salvo cabeceras descriptivas/includes aditivos; ninguna llamada productiva nueva.
No migración de B01. Establecimiento≤20 pasos a±2RPM confirmado por usuario;
S8 conserva planta200RPM al100%, polo3/4 y retardo1paso. No es sintonía real.
Denominadores1..1000 intactos; detección de sensor desconectado fuera de alcance.
Propuesta aprobada r2; H4/H5 y m1 incorporados. Implementación verificada, pendiente de auditoría.

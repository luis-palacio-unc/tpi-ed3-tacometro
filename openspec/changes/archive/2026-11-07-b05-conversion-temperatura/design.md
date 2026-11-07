# Diseño B05 corregido — Separación servicio/dispositivo

## Decisión y fuentes

La aclaración actual del usuario refuerza R15: servicios también independientes del
hardware específico. Temperatura.c de fcd4392 contenía el factor LM35; LM35.h delegaba
la conversión al servicio. Son fuentes previas contradictorias con la aclaración actual,
que prevalece. Corregir B05 aún no aceptado; conservar historial/commit y evidencia inicial.
Datasheet local confirmado LM35 TI SNIS159H, §7.3.1 p.13: ganancia 10 mV/°C.

| Opción | Compromiso | Decisión/motivo |
|---|---|---|
| Curva fija o configurable en servicio / convertir en dispositivo | Parametrizar ganancia mantiene servicio ligado a modelo eléctrico | Curva y ADC→Celsius en Dispositivos/LM35 |
| Callback/proveedor activo / entrada de datos físicos | No hay adquisición real en B05; callback agrega ciclo de vida sin requisito actual | Servicio recibe Celsius/estado por valor, fuente intercambiable |
| Datos específicos compartidos / contratos propios | Compartir enum LM35 acopla servicio al sensor | LM35_Lectura propia; Temperatura usa estados de fuente genéricos |
| Temperatura sin signo / int32_t | Celsius puede ser negativo en otra fuente | Servicio int32_t físico; LM35 conserva uint32_t sin offset, siempre <=42949673 |
| Cambiar algoritmo / trasladar y verificar | Algoritmo numérico ya verificado | Mantener cociente/resto y límites; mover pruebas a dispositivo |

## APIs y flujo

Dispositivos/LM35.h/.c, ESW.2.1.11:
LM35_InicializarLectura(.1) reinicia registro, no hardware; LM35_ConvertirMuestra(.2)
recibe muestra, escala_adc y referencia_uv uint32_t. Devuelve LM35_Lectura con
centesimas_c uint32_t y estados SIN_DATOS/VALIDA/errores de argumento, escala,
referencia y muestra. Misma prioridad y cálculo que B05 inicial; sin registros/DMA.

Servicios/Temperatura.h/.c, ESW.2.1.4:
Temperatura_Inicializar(.1) deja SIN_DATOS/cero.
Actualizar(.2) recibe salida, int32_t centesimas_c y Temperatura_EstadoFuente.
FUENTE_VALIDA copia valor/VALIDA; FUENTE_SIN_DATOS limpia/SIN_DATOS; FUENTE_ERROR
limpia/ERROR_FUENTE; enum desconocido limpia/ERROR_ARGUMENTO. NULL devuelve
ERROR_ARGUMENTO sin acceso (false al inicializar). No lee estado anterior.
No hay parámetros ADC, ganancia ni inclusión/llamada LM35 en el servicio.

Fuente específica → magnitud Celsius/estado → servicio genérico → aplicación.
La adaptación de integración se demuestra en test/fixture, no en main. Mapea LM35
VALIDA/SIN_DATOS/otros a FUENTE_VALIDA/SIN_DATOS/ERROR; convierte uint32_t a int32_t
sólo tras comprobar validez y cota (máximo 42949673<INT32_MAX). Futuro ensamblado
de adquisición proporcionará estos datos sin introducir acceso directo a dispositivos
en la lógica de aplicación. Sin composición física nueva en B05.
Servicio admite negativos de otra fuente; no implementa circuito negativo del LM35.
Lectura/conversión/acceso requieren serialización externa; sin detección de frescura.

## Aritmética y archivos

En LM35: N=muestra×referencia_uv, D=escala_adc×100; cociente/resto para redondear.
N<=18446744060824649730, D<=429496729500; nunca sumar D/2 al numerador. Salida
<=42949673. Servicio sólo copia int32_t y estados, sin cálculos eléctricos ni runtime64.
Sin heap/globals/esperas. Medir ambas unidades, pila y enlaces ARM por separado.

Modificar dos pares .h/.c, CMakeLists y Doxyfile. test_temperatura prueba servicio
sin incluir/enlazar dispositivos; test_lm35 conserva 17 casos aritméticos;
test_integracion_temperatura prueba adaptación. Fixture B05 de servicio independiente;
nuevo fixture LM35+servicio retiene APIs y auxiliares libgcc. Actualizar documentos;
verify-report-inicial.md conserva evidencia fcd4392 sin acreditarla para interfaz nueva.
No modificar contratos/historial aceptados ni otros módulos físicos.

## Verificación y límites

Reproducir primero fallo de compilación de consumidor Celsius/estado contra interfaz
previa. Después ejecutar servicio aislado, LM35, integración, sanitizadores, regresiones,
ARM Debug/Release y Doxygen (cuatro USW de estas unidades). Pruebas de saturación,
equivalencia, redondeo y extremos se mantienen, sin recalcular cociente en oráculo.
R15 se aplica transversalmente: B03 conserva 60 pulsos/vuelta en contrato aceptado;
registrar revisión posterior, sin modificarlo dentro de B05. No declarar toda capa
de servicios desacoplada aún. Pines/VREF/variante/rango y tiempos reales pendientes.

# B07 — Coordinación portable de salidas de alarma

Estado: resultado aceptado por usuario el 2026-11-09. Sincronización, archivo y
publicación de commits pendientes autorizados; propuesta B08 solicitada.

## Objetivo y origen

R07 requiere LED rojo y buzzer; R10/R15 separan aplicación, servicios y dispositivos.
B06 decide la condición térmica. Proponer Driver_Alarmas como intermediario que
envía solicitudes lógicas a dos salidas reemplazables, sin conocer placa/pines/polaridad.

## Alcance y comportamiento propuesto

- Recibir solicitud genérica activar/desactivar; no recibir temperatura ni decidir umbral.
- Configurar interfaz de dos funciones de salida y su contexto externo, sin heap,
  registro global o marco adicional. Las implementaciones físicas llegarán después;
  el contrato exige operaciones que retornen sin esperas activas.
- Activar solicita LED y buzzer activos; desactivar solicita ambos inactivos.
  Intentar cada canal una vez por llamada, incluso si el otro informa fallo.
- Validar interfaz completa antes de emitir órdenes. Argumentos/interfaz inválidos
  producen error sin llamadas; informar fallo LED, buzzer o ambos sin declarar éxito parcial.
- El éxito acredita aceptación por las funciones de salida, no nivel eléctrico real.
- Probar adaptación de B06: sólo ACTIVA/INACTIVA emiten solicitudes; ausencia/error
  no se traducen a apagado ni emiten órdenes. Conservar su diagnóstico observable.
  Adaptación en pruebas, sin dependencia Servicios→Aplicacion ni main integrado.

Excluye GPIO/LED/Buzzer físicos, pines, polaridad, patrón físico ante fallo de lectura,
parpadeo, histéresis, enclavamiento, reintentos, timeout, prioridad temporal, motor,
adquisición, UART y main. No cambiar la decisión térmica ni los contratos aceptados.

## Dependencias y archivos

B00–B06/B03-R1 aceptados. Servicios/Driver_Alarmas.h/.c, pruebas host con salidas
emuladas y adaptación B06, fixture ARM, CMake/Doxyfile y seguimiento SDD.
Documentar ESW.2.1.6/USW, argumentos, errores y serialización; sin dependencias nuevas.
Antes del código: especificación/diseño/tareas del alcance acordado.

## Criterio de aceptación

1. Activar/desactivar/repetir entrega el estado solicitado a ambos canales, una llamada
   por canal; dos instancias con contextos distintos no se interfieren.
2. Interfaz inválida no produce llamadas; fallos de cada canal/ambos son observables
   y una solicitud posterior válida recupera sin estado de error retenido.
3. Integración B06 emite órdenes sólo para decisiones evaluables; servicio compila
   sin aplicación, dispositivos concretos o registros LPC1769.
4. Host/sanitizadores, regresiones, enlace ARM y Doxygen estricto pasan con evidencia;
   aceptación física de LED/buzzer permanece pendiente.

## Riesgos y reversión

Riesgo: confundir éxito lógico con confirmación física o silenciar lectura inválida.
Mitigar con contratos/estados explícitos y pruebas de llamadas/fallos. Reversión futura:
revertir sólo el commit del bloque conservando cambios ajenos e historial.

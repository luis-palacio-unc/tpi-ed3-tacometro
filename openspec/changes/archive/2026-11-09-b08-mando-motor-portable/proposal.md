# B08 — Mando de motor portable

Estado: resultado aceptado el 2026-11-09. Sincronización, archivo y publicación
autorizados; propuesta B09 solicitada sin implementación.

## Objetivo y origen

R03 requiere que el mando manual determine PWM; R06 exige salida cero al entrar
en automático hasta una nueva consigna. R10/R15 exigen portabilidad.
B01/B04 entregan fuente y mando normalizado.

## Alcance y comportamiento propuesto

- Recibir mando lógico de 0..1000 por mil, usando el límite compartido existente;
  0 es 0 % y 1000 es 100 %.
- Proveer aplicación del mando y operación explícita de solicitar cero. La operación
  de cero solicita 0 por mil; no acredita detención física.
- Usar función de salida reemplazable con contexto externo, sin registro global,
  heap o espera propia. El proveedor traducirá por mil a su PWM físico posteriormente.
- Validar interfaz y rango antes de enviar. Fuera de rango se rechaza con error,
  sin saturar ni emitir una orden; interfaz inválida tampoco llama al proveedor.
  Fallo declarado por la salida es observable y la siguiente solicitud válida recupera.
- Integración de prueba: fuente CERO B01 solicita cero; MANUAL usa el mando válido
  B04/B01. Fuente AUTOMATICA queda para el regulador posterior, sin enviar la
  consigna RPM como PWM. Conservar B01/B04.

Excluye dispositivo Motor/PWM real, pines/frecuencia/reloj, etapa de potencia,
inversión/frenado, PI/PID, rampas, main, ADC, UART, alarmas y políticas nuevas de parada.
Aceptación por proveedor no confirma forma de onda, giro o velocidad física.

## Dependencias y archivos

B00–B07/B03-R1 aceptados. Servicios/Driver_Motor.h/.c, pruebas host con salida
emulada/adaptación de prueba B01/B04, fixture ARM, CMake/Doxyfile y seguimiento SDD.
Documentar ESW.2.1.8/USW, unidades, errores y serialización.
Especificación/diseño/tareas tras acordar alcance; Motor físico pendiente.

## Criterio de aceptación

1. Verificar 0/intermedios/1000 y solicitar cero; cada solicitud válida llama una vez
   al proveedor con el valor exacto, sin estado de instancia compartido.
2. Fuera de rango e interfaz inválida no emiten órdenes; fallo de salida y recuperación
   son observables. Probar límites de tipos, interfaz intacta y paso del contexto exacto.
3. Probar manual→automático con cero y preservación de B01/B04; servicio compila
   aislado de aplicación, dispositivos concretos y registros del microcontrolador.
4. Host/sanitizadores, regresiones, enlace ARM y Doxygen estrictos pasan; evidencia
   separada de PWM/etapa de potencia/mediciones físicas aún pendientes.

## Riesgos y reversión

Riesgos: confundir por mil con RPM o rechazo con parada efectiva. Mitigar mediante
contratos/estados y pruebas de órdenes. Reversión futura: revertir sólo el commit
del bloque, preservando trabajo ajeno e historial.

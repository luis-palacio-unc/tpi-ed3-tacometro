# B01 — Gestión de modos y selección del mando

Estado: alcance y resultado, incluido máximo 185 RPM, aceptados el 2026-11-06.

## Objetivo y requisitos de origen

Implementar y probar en host la selección exclusiva de la fuente de mando y la
espera de una consigna nueva al entrar en automático. Origen: R05/R06 del
anteproyecto; prepara R03/R04 sin completar PWM físico ni regulación de RPM.

## Alcance

- Estados MANUAL, AUTO_ESPERA_CONSIGNA y AUTO_ACTIVO en Procesamiento.
- Inicialización, cambios de modo, aceptación/descarte de consignas y selección
  exclusiva de la ruta manual o automática.
- Entradas: modo lógico, mando manual y eventos de consigna con validez y antigüedad.
- Salidas: estado, fuente habilitada, consigna vigente y orden de salida cero mientras
  se espera consigna. AUTO_ACTIVO habilita el control futuro; no calcula aún su PWM.
- Reiniciar la consigna al entrar en automático y evitar reutilizar datos en cola
  recibidos antes de esa entrada, aunque se procesen después.

Excluye pines, antirrebote físico, ADC/DMA, UART real/protocolo, PWM físico, reloj,
PID/PI, alarmas e integración del lazo principal con hardware.

## Dependencias y decisiones pendientes

B00 aceptado. Antes de implementar, especificar unidades/tipos, orden de eventos
simultáneos, identificación de consignas nuevas y respuesta a entradas inválidas.
La antigüedad se refiere a recepción, no al orden de procesamiento.
No hace falta conocer pines para verificar esta lógica aislada.

## Archivos previstos

Aplicacion/Procesamiento.c/.h; nuevos tests/host/ y configuración CTest/CMake.
Encabezados de servicios sólo si el diseño necesita definir sus contratos lógicos.

## Escenarios y aceptación

1. Arranque manual: sólo se habilita la ruta del potenciómetro.
2. Arranque o transición a automático: salida cero hasta consigna nueva válida.
3. Consignas recibidas en manual, antiguas o pendientes de una entrada anterior:
   no habilitan automático ni se recuperan al cambiar de modo.
4. Nueva consigna válida: AUTO_ACTIVO; cambios del potenciómetro no alteran su mando.
5. Vuelta a manual y nueva entrada a automático: se exige otra consigna nueva.
6. Consignas inválidas no habilitan ni reemplazan la consigna activa.

Aceptación: escenarios probados ejecutando C en host, incluidos límites/errores del
contrato; compilación ARM y pruebas B00 sin regresiones; revisión de capas y diff.
Evidencia: tests/resultados y matriz requisito–escenario en verify-report.md.
No se declara funcionamiento físico ni regulación real por estas pruebas.

## Riesgo y reversión

Riesgo: aceptar mensajes viejos por una interfaz sin antigüedad. Resolverlo en diseño
antes de código. Revertir sólo el diff propio B01; conservar B00 y cambios ajenos.

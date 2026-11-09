# Estado — 2026-11-09

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD con git log -1.
- B00–B06 y B03-R1 aceptados/archivados. B06 aceptado el 2026-11-09;
  contrato vigente alarma-temperatura sincronizado (cuatro requisitos/ocho escenarios).
- B06 implementación 47ae9b5e71d9ee233e5b99ac7adb0f4472c468e5; cierre documental
  en commit `docs: aceptar y archivar B06`. Hooks/firma/historial conservados.
- Usuario autorizó publicar commits pendientes y cierre B06. Ejecutar push y comprobar
  remoto antes de preparar B07. Base remota conocida a11a97c1c8617fdc42007afa78d071e83b553512.
- Propuesta B07 pendiente de preparar; no hay autorización de implementación.

## Evidencia aceptada

- B06: 115 casos C (13 B06), 11199 ternas B03-R1 y sanitizadores PASS;
  ARM Debug 9/9, B06 Release 1/1, Doxygen estricto 22 funciones/USW.
- Matriz AT1–AT4/S1–S8 y recursos en informe archivado B06.
- Cierre sólo documental: fusión exacta, preservación de fuentes/historial, manifiesto,
  YAML/enlaces y diff. No se ejecutan ni declaran nuevas pruebas de firmware.
- Sin ejecución ARM, simulación o hardware; sin instalaciones/flasheo/cambios MCU.
- Logs cierre: /tmp/ed3-cierre-b06-20261109-gqz_d7bw; logs originales en informe B06.

## Pendientes y próxima acción

Publicar commits autorizados y preparar alcance/criterio B07 para Driver_Alarmas:
solicitudes lógicas hacia salidas LED/buzzer reemplazables, verificables con emulación.
Conservar decisión de umbral en aplicación y no introducir Servicios→Aplicacion.
GPIO/pines/polaridad, señales reales, respuesta física ante lectura fallida y prioridad
temporal siguen pendientes, al igual que adquisición, UART, PWM/control y main.
CMSIS-DAP integrado; revisión física PCB/silicio y montaje/pines/VREF sin verificar.
No implementar B07 ni ampliar publicación a trabajo posterior sin acuerdo.

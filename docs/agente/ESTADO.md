# Estado — 2026-11-07

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; comprobar HEAD con git log -1.
- B00–B05 aceptados/archivados. B05 corregido aceptado, contrato sincronizado y
  commits fcd4392/a1f3b46/a11a97c publicados. origin/main=a11a97c1c8617fdc42007afa78d071e83b553512.
- Revisión B03-R1 autorizada en la misma orden, implementada/verificada y pendiente
  de aceptación. Cambio activo b03-factor-pulsos-configurable; contratos/archivos aceptados intactos.
- Commit local con asunto `fix: parametrizar pulsos por vuelta en medicion de velocidad`;
  consultar Git para hash. Sin push nuevo autorizado. Conservar hooks/firma e historia.
- No iniciar B06. main mínimo; sin integración física.

## Evidencia actual ejecutada

| Verificación | Resultado |
|---|---|
| Host y sanitizadores | CTest 8/8, 102 casos C (23 B03) + 11199 ternas de oráculo; sin hallazgos |
| Regresión B03 | Factor 60 conserva 15 casos anteriores, 100 fronteras y 10000 ventanas |
| Generalización | Factores 1/12/60/100/1024/MAX, prioridades/recuperación y 6150 puntos monótonos |
| ARM | Debug 8/8, B03 Release 1/1; pruebas finales B03 tras claridad de unidades PASS |
| Doxygen | Estricto, sin avisos; 21 funciones/USW, dos B03 y configuración documentadas |
| Aislamiento | Mismo consumidor que fallaba compila sin configuración/dispositivos; oráculo PASS |
| Recursos -O0 | Código 828, .data/.bss 0; pila propia 224, cadena libgcc hasta 272 bytes |
| Preservación | Aplicación/dispositivos/bibliotecas y todos los contratos/archivos aceptados intactos |

Matriz S1–S10, comandos y recursos: verify-report.md del cambio activo.
Logs: /tmp/ed3-b03-r1-20261107-6hxxb4jm, fuera del árbol versionado.
Sin fallos funcionales pendientes; sin ejecución ARM, simulación o prueba física.
No se instaló, flasheó ni modificó configuración sensible del MCU.

## Decisiones y limitaciones

Factor pulsos/vuelta positivo por llamada; valor 60 del disco sólo en configuración
TPI, sin dependencia del servicio hacia ese archivo. El 60 interno de unidades es
segundos/minuto. Firma nueva de cuatro argumentos; consumidores migrados, sin wrapper.
Estado/valor y redondeo se conservan; factor cero invalida con error propio.
Captura/configuración coherentes y acceso serializado externos. Todavía sin detección
de frescura, captura, ADC/DMA, UART, PWM/control, alarma o main integrado.
CMSIS-DAP integrado; revisión PCB/silicio, pines/VREF/montaje pendientes.

## Próxima acción

Revisar aceptación de B03-R1. Sólo después sincronizar/archivar con orden del usuario.
No publicar la revisión ni iniciar otro bloque sin autorización.

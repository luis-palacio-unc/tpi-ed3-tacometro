# Estado — 2026-11-09

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD con git log -1.
- B00–B05 y B03-R1 aceptados/archivados. Base de B06: dd80d582e880b4867fe50019b43a5c1b5908daf3.
- B06: alcance acordado, implementado/verificado, pendiente de aceptación del resultado.
  Cambio activo b06-logica-alarma-temperatura; contratos principales/archivo sin cambios.
- Commit local con asunto `feat: evaluar alarma de temperatura configurable`;
  consultar Git para hash. Sin push autorizado. Conservar hooks/firma e historial.
- Última base remota conocida origin/main=a11a97c1c8617fdc42007afa78d071e83b553512.
  Revisión B03-R1, cierre/propuesta y B06 permanecen locales.

## Evidencia actual ejecutada

| Verificación | Resultado |
|---|---|
| Host/sanitizadores | CTest 9/9; 115 casos C (13 B06) + 11199 ternas B03-R1, sin hallazgos |
| ARM | Debug 9/9 (B00 conserva 14 casos), B06 Release 1/1; APIs retenidas, ELF válidos |
| Doxygen | Estricto sin avisos, 22 funciones/USW; nueva USW.2.1.2.7 |
| Controles negativos | Suite detecta >= incorrecto y ausencia tratada como INACTIVA |
| Nueva evaluación -O0 | Código 78, pila propia 16 bytes, sin auxiliares externos |
| Preservación | Seis cuerpos/tipos/firmas B01 idénticos; servicios/dispositivos/contratos/archivos aceptados intactos |

Matriz AT1–AT4/S1–S8, comandos, recursos y límites: verify-report.md activo B06.
Logs: /tmp/ed3-b06-20261109-baw2h0g0, fuera del árbol versionado. Sin fallos funcionales pendientes.
El control de tamaños inicial suponía enum de 4 bytes; sizeof/símbolos ARM miden 1,
coinciden tras corregir el comprobador. No se cambiaron ABI/flags o firmware por esto.
Sin ejecución ARM, simulación o prueba física. No se instaló ni flasheó nada.

## Decisiones y limitaciones

Aplicación consume const Temperatura_Lectura* y umbral int32_t en centésimas Celsius
configurable sin default. Retorno completo ACTIVA/INACTIVA/SIN_DATOS/ERROR_FUENTE/
ERROR_ARGUMENTO. Sólo VALIDA evalúa T>umbral; igualdad inactiva, errores no son normalidad.
Sin contexto/inicialización nuevos ni modificación de B01/motor. El llamador asigna
el retorno y serializa lectura. Driver_Alarmas/LED/Buzzer y main siguen pendientes;
sin prioridad temporal, histéresis, timeout, enclavamiento o política física ante fallo.
ADC/DMA, captura, UART, PWM/control y montaje/pines/VREF también pendientes.
CMSIS-DAP integrado; revisión física PCB/silicio sin verificar.

## Próxima acción

Revisar aceptación del resultado B06. Sólo después sincronizar/archivar por orden.
No publicar ni iniciar B07 sin autorización.

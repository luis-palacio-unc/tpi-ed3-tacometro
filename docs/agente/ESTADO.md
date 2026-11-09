# Estado — 2026-11-09

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD con Git.
- B00–B06 y B03-R1 aceptados/archivados. B06 y su cierre/propuesta publicados;
  base conocida de B07: eb713e04cebd93068c08161cd67af9ede31d1388.
- B07: alcance acordado, implementado/verificado, pendiente de aceptación del resultado.
  Cambio activo b07-coordinacion-salidas-alarma; contratos vigentes/archivo sin cambios.
- Commit local con asunto `feat: coordinar salidas de alarma mediante interfaz portable`;
  consultar Git para hash. Sin push autorizado; hooks/firma/historial conservados.

## Evidencia actual ejecutada

| Verificación | Resultado |
|---|---|
| Host/sanitizadores | CTest 11/11; 131 C (12 servicio + 4 integración B07) y 11199 ternas B03-R1, sin hallazgos |
| ARM | Debug 10/10, B07 Release 1/1; APIs/proveedor retenidos, ELF válidos |
| Doxygen | Estricto sin avisos; 23 funciones/USW, nueva USW.2.1.6.1 y callback documentados |
| Negativos | Suite detecta buzzer siempre activo y cortocircuito tras fallo LED |
| Recursos -O0 | Código 144; pila propia 24 bytes, .data/.bss 0; interfaz 16, enum 1 con toolchain actual |
| Aislamiento/preservación | Servicio sólo necesita Servicios/std; aplicación, dispositivos, otros servicios, pruebas previas y contratos/archivos aceptados intactos |

Matriz SA1–SA4/S1–S8, comandos y límites: verify-report.md activo B07.
Logs: /tmp/ed3-b07-20261109-v_30cvtq, fuera del árbol versionado. Sin fallos funcionales pendientes.
Sin ejecución ARM, simulación o hardware; sin flasheo/instalación/cambios MCU.

## Decisiones y limitaciones

Interfaz por llamada, funciones/contextos externos separados. Validación completa
previa; LED→buzzer una vez cada uno aunque falle LED. ACEPTADA/error de argumento/
LED/buzzer/ambos son resultados de llamada, no estado eléctrico confirmado.
Sin heap/globals mutables/init/cache/reintentos/timeout; proveedores deben retornar,
interfaz/contextos estables/vivos y acceso serializado. Pila/tiempo total dependen de ellos.
Adaptación B06 sólo en pruebas/fixture, sin Servicios→Aplicacion. Sólo ACTIVA/INACTIVA
emiten solicitudes; ausencia/error no emite apagado y conserva diagnóstico.
GPIO/pines/polaridad, patrón físico de fallo y prioridad temporal siguen pendientes,
al igual que adquisición, UART, PWM/control y main. CMSIS-DAP integrado;
revisión física PCB/silicio y montaje/pines/VREF sin verificar.

## Próxima acción

Revisar aceptación del resultado B07. Sólo después sincronizar/archivar por orden.
No publicar ni iniciar B08 sin autorización.

# Estado — 2026-11-09

## Punto actual

- Repositorio: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD con Git.
- B00–B08 y B03-R1 aceptados/archivados. Base publicada B09: 98635281ac6f79204a01dcf2946c92479e4824bf.
- B09: alcance/formato/criterio acordados, implementado/verificado; resultado pendiente de aceptación.
  Cambio activo b09-codificacion-telemetria. Contratos vigentes/archivos aceptados intactos.
- Commit local autorizado con asunto `feat: codificar telemetria ASCII portable`;
  consultar Git para hash. Sin push autorizado; conservar hooks/firma/historial.

## Evidencia ejecutada

| Verificación | Resultado |
|---|---|
| Host/sanitizadores | CTest 15/15, 161 C (11 codec + cuatro integración B09), 5096 pares codec y 11199 ternas B03-R1; sin hallazgos |
| ARM | Debug 12/12, fixture B09 Release PASS; API B09/B01/B03/B05/RX retenida y ELF válido; sin ejecución |
| Doxygen | 29 funciones/USW, cuatro nuevas USW.2.1.7.7–.10; sin avisos |
| Negativos | Detecta pérdida de padding decimal y error de capacidad para NUL |
| Recursos -O0 | Codec/helpers .text 892 + rodata 65, sin .data/.bss; pila propia anidada 176 excluye llamador/IRQ/otros módulos; instantánea 12, enum 1 con ABI actual |
| Preservación | Cuerpos/tipos/enums/constantes RX B02, Procesamiento/main, otros servicios, dispositivos, Drivers, pruebas previas, startup/linker y contratos/archivos aceptados intactos |

Matriz CT1–CT4/B09-S1–S8 y B02-R5/S9 en verify-report.md activo. Logs externos: /tmp/ed3-b09-20261109-wdrfl0hk.
Sanitizadores ejecutados sin ptrace con LeakSanitizer habilitado. Último ajuste a
helpers fue sólo documental; Doxygen repetido sin avisos, cuerpos probados intactos.
Sin ejecución ARM, simulador o placa; sin flasheo/instalación/cambios sensibles MCU.

## Próxima acción y límites

Revisar resultado B09; sólo después sincronizar/archivar por orden. No publicar ni
iniciar otro bloque sin autorización. Aplicación reunirá datos; adaptación ahora
sólo en pruebas/fixture, nunca Servicios→Aplicacion. UART/envío/periodicidad/main,
adquisición, PWM/potencia/regulación y pruebas físicas pendientes.
CMSIS-DAP integrado; revisión PCB/silicio, montaje/pines/VREF sin verificar.
Diagramas para informe después de completar todos los ESW, según CONTEXTO.

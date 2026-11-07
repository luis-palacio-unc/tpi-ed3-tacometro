# Estado — 2026-11-06

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama main; revisión actual: `git rev-parse HEAD`; sincronización: `git status -sb`.
- Implementación B02: 4e6c1d4. Usuario aceptó B02 y autorizó publicar este commit
  y el cierre documental con propuesta B03 el 2026-11-06.
- B00–B02 aceptados/archivados. Contratos build, modos y comunicación vigentes.
- Archivo B02: openspec/changes/archive/2026-11-06-b02-recepcion-consignas/;
  no modificar archivos archivados.
- B03 propuesto en openspec/changes/b03-medicion-velocidad/, sin acuerdo ni código.
- Commits al cerrar bloques acordados autorizados; push de nuevos bloques requiere
  permiso independiente. Firma/hook Git conservados.
- main sigue mínimo; UART, sensor y accionamiento físicos no integrados.

## Evidencia ejecutada

| Verificación | Resultado |
|---|---|
| Host CTest | 2/2; 17 casos B01 + 17 B02, incluye bytes 0..255 y FIFO/recuperación |
| ASan/LeakSanitizer/UBSan | 34/34 casos sin hallazgos, fuera del sandbox |
| ARM Debug CTest | 4/4; B00 14/14, fixture B01 y nuevo B02 |
| ARM Release | Build y enlace fixture B02 a -O3 correctos |
| Doxygen estricto HTML/XML | Sin avisos; 6 USW nuevas, 13 totales verificadas en XML |
| Recursos -O0 | Contexto Comunicación 88 bytes; código 818 bytes, sin heap/globals |
| Stack estático módulo | Hasta 64 bytes de llamadas internas; no medición en hardware |
| Preservación | main, Procesamiento.c, tests B01, drivers/dispositivos y archivos aceptados intactos |
| Copia limpia del índice al cerrar B02 | 34 casos host, CTest ARM 4/4 y Doxygen estricto correctos |

Logs: `/tmp/ed3-b02-20261106-6p_kq52o` (fuera del árbol versionado; temporales).
Cierre/publicación: /tmp/ed3-cierre-b02-20261106-bep0bv0r (resultados desde copia limpia del índice).
Matriz S1–S9 y comandos en el informe archivado de B02. Sin fallos funcionales
observados. No se ejecutó firmware ARM en MCU/simulador, no se flasheó ni instaló nada.

## Decisiones y limitaciones actuales

- Recepción lógica: ASCII RPM, un espacio, 1..3 dígitos, LF/CRLF; 0..185 RPM,
  ceros iniciales permitidos. Rechazo explícito; no saturar valores inválidos.
- Buffer 8 bytes antes de LF y cola FIFO de 4 eventos, sin heap. Desborde de trama
  descarta hasta LF; cola llena conserva anteriores y rechaza nuevo con error.
- Cada byte lleva orden global único creciente; evento conserva marca del LF. Captura
  y llamadas serializadas por integrador; nunca usar orden enviado desde PC/extraído
  de cola. Reinicio purga cola/trama y requiere coordinación de sesión con B01.
- Máximo común en Configuracion/Limites.h; B01 conserva alias público a 185 RPM.
- Faltan integración UART/ISR, telemetría, pines/revisión PCB/silicio, VREF, reloj,
  depuración física, sensores, PWM/regulador, alarmas y lazo principal.
- No hay CI ni aceptación física. No consta aprobación formal del anteproyecto aquí.
- Doxyfile parcial cubre main, Procesamiento, Comunicacion y límites compartidos.
  Continuar documentando ESW/USW por bloque; no es documentación final completa.
- Configuración global Git mantiene AGENTS/.atl locales; procedimientos en docs/agente/.

## Próxima acción

Acordar alcance y aceptación B03 antes de implementar; fijar representación/redondeo
en su especificación. Al reanudar, verificar HEAD/upstream; no inferir permiso de push.

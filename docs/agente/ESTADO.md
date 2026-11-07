# Estado — 2026-11-06

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama main; base B02 1535715. Obtener revisión actual con `git rev-parse HEAD`.
  Consultar `git status -sb`: el commit B02 queda local, sin push.
- Usuario autorizó B02 completo, Doxygen y commit al terminar, sin push.
  Continúa autorización de commits al cerrar bloques acordados; hooks/firma sin cambios.
- B00/B01 aceptados y archivados; contratos build/modos vigentes en openspec/specs/.
- B02 implementado/verificado; resultado pendiente de aceptación. Artefactos activos
  en openspec/changes/b02-recepcion-consignas/. No sincronizar ni archivar aún.
- No se inició B03. main sigue mínimo; servicios aún no integrados con UART física.

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

Logs: `/tmp/ed3-b02-20261106-6p_kq52o` (fuera del árbol versionado; temporales).
Matriz S1–S9 y comandos en verify-report.md del cambio B02. Sin fallos funcionales
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

Revisar B02 con el usuario. Tras aceptación, sincronizar/archivar y acordar próximo
bloque. Push requiere autorización independiente; no iniciar B03 automáticamente.

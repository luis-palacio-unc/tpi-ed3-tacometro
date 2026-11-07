# Estado — 2026-11-06

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama `main`; identificador de revisión: consultar `git rev-parse HEAD` y
  `git status --short --branch` al reanudar (no incrustar el propio hash en el commit).
- Origin privado: `https://github.com/luis-palacio-unc/tpi-ed3-tacometro.git`.
- Usuario aceptó B00 y autorizó commit/push de esta estructura inicial el 2026-11-06.
  Esta autorización no se extiende a bloques posteriores.
- B00 aceptado, especificación vigente en `openspec/specs/build/spec.md` y cambio
  archivado en `openspec/changes/archive/2026-11-06-b00-build-lpc1769/`.
- B01 propuesto en `openspec/changes/b01-gestion-modos/`, sin implementación.
- Se respeta configuración de firma de Git: sin firma automática habilitada.
- Sincronización remota: comprobar `git status -sb` y `git ls-remote origin refs/heads/main`
  al reanudar; no depender del estado recordado en el chat.

## Evidencia ejecutada

| Comprobación | Resultado | Alcance |
|---|---|---|
| GCC ARM 13.2.1 / CMake 3.28.3, Debug | ELF, mapa, memoria.txt válidos | Compilación cruzada e inspección |
| CTest Debug | 2/2 pruebas; suite de contratos 14/14 casos | Herramientas/build/ELF ejecutados en host |
| Build Release desde directorio limpio | Correcto, verificación automática correcta | Optimizaciones y enlace ARM |
| Build host dentro de la suite | Correcto, 18 objetos | Sin ejecución de main |
| Fixture .data/.bss y revisión del startup | Correcto; llamada a main y límites coherentes | Inspección estática; no ejecución ARM |
| Presets CMake | Host y LPC1769 reconocidos | Configuración |
| Preservación | Drivers, servicios y dispositivos intactos | SHA-256 contra inicio B00 |
| Copia limpia del índice para publicación | Build ARM correcto; CTest 2/2 y suite 14/14 | Sin archivos locales excluidos |

Debug: huella flash 844 bytes; SRAM local asignada 2048 bytes (reserva de pila).
Release: huella flash 840 bytes; misma reserva. No acredita profundidad real de pila.
Artefactos/logs: `/tmp/ed3-b00-20261106-yyj_xnlc` (temporales, fuera del proyecto).
Reproducción y matriz S1–S7 en el informe archivado; comandos de uso en README.
Logs de publicación/copia limpia: `/tmp/ed3-publicacion-20261106-wu5a4yvx` (fuera del árbol versionado).

Se corrigieron durante B00: nota GNU-stack ausente, comparación de nombre cosmético
CPU y reescritura de fecha del ELF por objcopy. Se conservaron los logs de los fallos.
No se ejecutaron unitarias funcionales del tacómetro, simulación ni pruebas físicas.

## Limitaciones y pendientes

- Sonda elegida: CMSIS-DAP integrado. Revisión PCB/silicio sin confirmar.
- Sin sesión de depuración configurada ni GDB ARM/OpenOCD/pyOCD/LinkServer detectados
  en PATH durante incorporación. No se instalaron dependencias ni se flasheó.
- Boot físico, consumo real de pila, reloj y periféricos aún no validados.
- Faltan conexiones/VREF, motor/driver, límites RPM, temporización, protocolo UART,
  umbral y recuperación de alarma. No consta aprobación formal del anteproyecto.
- La biblioteca CMSIS no se modificó; SystemInit y las suites heredadas no se ejecutan.
- Sin CI. Sólo se autorizó publicación inicial; no se modificaron claves, firma o hooks.
- `git diff --cached --check` inicial: 205 avisos heredados, todos de Drivers. Se
  conserva biblioteca intacta y log completo; código propio/documentación pasan.
- AGENTS.md sigue excluido por ignore global del usuario; no se forzó su inclusión.
  El procedimiento y contexto compartidos están versionados en docs/agente/.

## Próxima acción

Acordar alcance y criterio de aceptación de B01 antes de implementarlo. Al reanudar,
comprobar HEAD/upstream; cualquier commit o push de otro bloque requiere autorización.

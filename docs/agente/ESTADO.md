# Estado — 2026-11-06

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama main; revisión actual: `git rev-parse HEAD`; sincronización: `git status -sb`.
- Commits de B01: a83d87b (implementación) y 4c8038a (máximo 185 RPM).
- B00/B01 aceptados y archivados. Contratos vigentes build y modos en openspec/specs/.
- B01: `openspec/changes/archive/2026-11-06-b01-gestion-modos/`; no modificar archivo.
- B02 propuesto en `openspec/changes/b02-recepcion-consignas/`, sin acuerdo ni código.
- Usuario autorizó commits al finalizar bloques acordados y push de los pendientes
  B01 más este cierre; no habilita push de bloques posteriores.
- Firma Git sin cambios (sin firma automática), hooks conservados.
- Doxygen incremental: seis USW Procesamiento y main documentadas; ampliar por módulo.
- main sigue mínimo; ninguna aceptación física de sensores/motor/UART.

## Evidencia realmente ejecutada

| Verificación | Resultado |
|---|---|
| C host Debug + CTest | 1/1; 17/17 casos funcionales |
| ASan/LeakSanitizer/UBSan | 17/17, sin hallazgos, fuera del sandbox |
| ARM Debug + CTest | 3/3; incluye contratos B00 14/14 y enlace API B01 |
| ARM Release | ELF y verificación estática correctos |
| Doxygen 1.9.8 HTML/XML estricto | Correcto sin avisos; 7 USW identificadas en XML |
| Doxygen sonda sin retorno | Rechazo esperado; controles habilitados |
| Recursos ARM -O0 | Contexto 40 bytes; módulo sin heap/globals; camino de llamadas hasta 72 bytes estáticos |
| Preservación | Drivers/Servicios/Dispositivos/archivo B00 intactos |
| Copia limpia del índice al cerrar B01 | C host 17/17, CTest ARM 3/3 y Doxygen estricto correctos |

Logs/artefactos: `/tmp/ed3-b01-20261106-02a8ln3f` (temporales, fuera del árbol versionado).
Última corrección: /tmp/ed3-rpm185-py52m3z4; 17/17 casos normales/sanitizados,
ARM CTest 3/3, Release y Doxygen correctos. La prueba nueva falló antes del cambio.
Cierre/publicación: /tmp/ed3-cierre-b01-20261106-cwceo_r4 (builds/pruebas/documentación desde copia del índice).
Detalle y matriz S1–S9/R14 en el informe archivado de B01.
Fallos iniciales registrados: LeakSanitizer incompatible con ptrace; Doxygen sólo XML
no reconocía correctamente @return. Se corrigió entorno/formato manteniendo controles.
No se ejecutó firmware ARM en placa o simulador ni se flasheó.

## Pendientes y limitaciones

- La secuencia lógica de recepción uint64_t no es tiempo; el integrador la asigna al
  recibir, conserva en cola y serializa captura/llamadas. No se admite rollover;
  reiniciar requiere inicialización y purga de colas. Esto aún no está integrado a ISR.
- Mando manual 0..1000 por mil; consignas aceptadas 0..185 RPM inclusive,
  comprobadas en Procesamiento además de la validación externa. Protocolo UART pendiente.
  La fuente automática habilita un regulador
  futuro, no calcula PWM en B01.
- Sin comprobación física de PCB/silicio, pines, VREF, reloj o sonda/configuración de
  depuración. CMSIS-DAP integrado elegido. Sin sesiones de debug configuradas.
- Faltan sensores, servicios, potencia, regulador, alarma y lazo principal integrados.
- Sin CI. No consta aquí aprobación formal del anteproyecto.
- Whitespace heredado de Drivers registrado en publicación B00; no se reformateó.
- AGENTS.md y .atl siguen locales según exclusiones; procedimiento compartido en docs/agente/.

## Próxima acción

Acordar alcance/protocolo y aceptación de B02 antes de implementar. Al reanudar,
verificar HEAD y upstream; no inferir permiso para push de nuevos bloques.

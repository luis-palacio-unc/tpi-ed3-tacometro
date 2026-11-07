# Estado — 2026-11-06

## Punto actual

- Proyecto: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`.
- Rama main. Base B01: a21cc1e; el commit actual se obtiene con `git rev-parse HEAD`.
  Consultar `git status -sb` al reanudar: el commit B01 queda local, sin push.
- Usuario autorizó commits al finalizar cada bloque acordado; no autorizó push B01.
  Se conserva configuración Git de firma (sin firma automática) y hooks.
- B00 aceptado/archivado; contrato vigente en openspec/specs/build/spec.md.
- B01 implementado y verificado, pendiente de aceptación del resultado.
  Artefactos en openspec/changes/b01-gestion-modos/. No sincronizar modos/archivar aún.
- No se inició B02. Procesamiento tiene API lógica; main sigue sin integrar servicios.
- Doxygen incremental: documentadas las 6 USW de Procesamiento y main; configuración
  parcial en docs/doxygen/Doxyfile. Continuar este estilo en bloques siguientes.

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

Logs/artefactos: `/tmp/ed3-b01-20261106-02a8ln3f` (temporales, fuera del árbol versionado).
Detalle y matriz S1–S9 en verify-report.md del cambio B01.
Fallos iniciales registrados: LeakSanitizer incompatible con ptrace; Doxygen sólo XML
no reconocía correctamente @return. Se corrigió entorno/formato manteniendo controles.
No se ejecutó firmware ARM en placa o simulador ni se flasheó.

## Pendientes y limitaciones

- La secuencia lógica de recepción uint64_t no es tiempo; el integrador la asigna al
  recibir, conserva en cola y serializa captura/llamadas. No se admite rollover;
  reiniciar requiere inicialización y purga de colas. Esto aún no está integrado a ISR.
- Mando manual 0..1000 por mil; RPM uint32_t validadas externamente. No se fijaron
  rango físico del motor ni protocolo UART. La fuente automática habilita un regulador
  futuro, no calcula PWM en B01.
- Sin comprobación física de PCB/silicio, pines, VREF, reloj o sonda/configuración de
  depuración. CMSIS-DAP integrado elegido. Sin sesiones de debug configuradas.
- Faltan sensores, servicios, potencia, regulador, alarma y lazo principal integrados.
- Sin CI. No consta aquí aprobación formal del anteproyecto.
- Whitespace heredado de Drivers registrado en publicación B00; no se reformateó.
- AGENTS.md y .atl siguen locales según exclusiones; procedimiento compartido en docs/agente/.

## Próxima acción

Revisar B01 con el usuario. Tras aceptación, sincronizar/archivar. Acordar el siguiente
bloque antes de implementarlo; push requiere autorización independiente.

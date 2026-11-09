# Estado — 2026-11-09

## Punto actual

- Repositorio: `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`. Rama main; consultar HEAD con Git.
- B00–B07 y B03-R1 aceptados/archivados. Base publicada B08: 501ab143ded9a59e4db39229733827b800213aab.
- B08: alcance/criterio acordados; implementado/verificado, resultado pendiente de aceptación.
  Cambio activo b08-mando-motor-portable. Contratos vigentes/archivos aceptados intactos.
- Commit local autorizado con asunto `feat: aplicar mando de motor mediante salida portable`;
  consultar Git para hash. Sin push autorizado; conservar hooks/firma/historial.

## Evidencia ejecutada

| Verificación | Resultado |
|---|---|
| Host/sanitizadores | CTest 13/13; 146 C (10 servicio + 5 integración B08) y 11199 ternas B03-R1; sin hallazgos |
| ARM | Debug 11/11, fixture B08 Release PASS; APIs/proveedor retenidos y ELF válidos, sin ejecución |
| Doxygen | Estricto sin avisos; 25 funciones/USW, nuevas USW.2.1.8.1/.2 y callback documentados |
| Negativos | Detecta saturación de mando inválido y cero equivocado |
| Recursos -O0 | Servicio .text 102, .data/.bss 0; pila propia Aplicar 16, ruta SolicitarCero 32 antes del proveedor; interfaz 8, enum 1 con ABI actual |
| Preservación | Aplicación, otros servicios/dispositivos, Drivers, pruebas previas, startup/linker y contratos/archivos aceptados intactos |

Matriz MM1–MM4/S1–S8, comandos y límites en verify-report.md activo B08.
Logs externos: /tmp/ed3-b08-20261109-qk7i0zj2. La primera corrida san falló por ptrace/LeakSanitizer;
repetición en proceso normal con todos los controles habilitados pasó 13/13; ambos resultados conservados.
Sin ejecución ARM, simulador o placa; sin flasheo/instalación/cambios sensibles MCU.

## Próxima acción y límites

Revisar aceptación del resultado B08; sólo después sincronizar/archivar por orden.
No publicar ni iniciar bloque posterior sin autorización.
B08 solicita mando/cero y reporta proveedor; adaptación B04/B01 sólo en pruebas.
PWM físico/pines/frecuencia/potencia, regulador, adquisición, UART y main pendientes.
CMSIS-DAP integrado; revisión PCB/silicio, montaje/pines/VREF sin verificar.
Diagramas de secuencia para el informe después de completar todos los ESW, según CONTEXTO.

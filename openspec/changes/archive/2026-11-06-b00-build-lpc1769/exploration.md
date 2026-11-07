# Exploración: incorporación y primer bloque

## Estado actual

Diagnóstico de sólo lectura ejecutado antes de crear documentos o repositorio.
No existían AGENTS, README raíz, docs/agente, openspec, Git local o CI del TPI.
Se leyeron instrucciones de ancestros y se buscaron instrucciones descendientes:
no se encontraron. Drivers/README y metadata Eclipse son de la biblioteca heredada.

Hay 18 .c y 17 .h propios con comentarios/guardas, sin funciones ni main definido.
56 .c y 62 .h en total incluyendo biblioteca/pruebas. CMake no usa Drivers,
no enlaza y no configura ARM. No hay startup ni linker script.
La biblioteca ofrece CMSIS y drivers de GPIO, timer, PWM, ADC, DMA y UART, entre otros.
Sus pruebas, habilitadas por UNIT_TESTING_ENABLED, escriben registros físicos;
no son pruebas host del tacómetro y no se ejecutaron. El README no prueba cobertura.

## Áreas afectadas

CMake y main deben evolucionar antes de obtener una imagen de placa. El soporte
CMSIS requiere selección/revisión: SystemInit configura PLL y flash y tiene esperas
sin timeout. No corresponde adoptar esa política por existir en la biblioteca.
La guía exige documentación ESW/USW; los esqueletos actuales no la completan.

## Alternativas

1. Implementar interfaces/lógica de aplicación primero: permite unitarias host,
   pero deja sin resolver cómo enlazar y verificar el firmware para la plataforma.
2. B00 de compilación ARM mínima: detecta temprano problemas de toolchain,
   memoria y arranque. Requiere cuidado con startup y alcance de la evidencia.

## Recomendación

B00 de build mínimo; después acordar el siguiente bloque, sin avanzar automáticamente.
CMSIS-DAP integrado confirmado por usuario; revisión de PCB/silicio pendiente.
La revisión D publicada por NXP no identifica automáticamente la unidad física.

## Evidencia del diagnóstico

CMake 3.28.3 y GCC host 13.3.0: configuración/build limpios correctos (18 objetos).
`nm --defined-only`: sin main. CTest --show-only=json-v1: 0 pruebas.
ARM GCC 13.2.1 presente, pero no se ejecutó compilación cruzada.
Logs: `/tmp/ed3-diagnostico-20261106-h4e7eg7g`.
Sin pruebas unitarias, simulación o ejecución sobre hardware. lsusb falló por libusb.
La consulta inicial de gh en sandbox no conectó; la consulta con acceso de red
confirmó autenticación y permitió crear/verificar repositorio privado vacío.

## Fuentes y riesgos

Fuentes: AP §1/anexos, guía REV 2.0, archivos actuales y documentación NXP
referenciada en CONTEXTO. Guía = formato/ejemplos; AP = alcance funcional.
No constan aprobación formal AP, conexiones, parámetros operativos o revisión física.
La URL directa consultada de UM10360 devolvió 404; obtener manual antes de B00.
No convertir documentación histórica o compilación de objetos vacíos en aceptación.

## Listo para propuesta

Sí. proposal.md define B00 para acuerdo; especificación/diseño/tareas/implementación
siguen pendientes. No hay contratos aceptados para sincronizar ni cambio que archivar.

Versiones de fuentes (SHA-256):
- anteproyecto.pdf: `051ad3e01f78c152d6b7c299bc539ddf4764d2b7e234efa6eb3f2e96a3e751c2`.
- GUIA_TPI_EDIII.pdf: `349b32be981726f01e91a367b4f831ead9ab9cb5eb8a339a3430c5df5add4357`.

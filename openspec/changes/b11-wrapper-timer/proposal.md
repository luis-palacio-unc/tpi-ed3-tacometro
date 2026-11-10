# B11 — wrappers del driver original

Alcance autorizado por usuario 2026-11-10: crear Timer en Dispositivos y borrar
Configuracion/Temporizacion.h. Origen R10/R15 y necesidad temporal del selector B10.
Configurar, arrancar y consultar un timer mediante API original; servicios reciben
ms para calcular intervalos. Tres wrappers cortos, sin modificar Drivers.

Archivos: Dispositivos/Timer.h/.c, CMake y documentación incremental. Base1ms
configurable en Timer.h. No recrear Tiempo, puerto, proveedor o driver extendido.
No integrar main, adquisición ni ventana RPM; sin flasheo/instalación/push.
Aceptación: tres wrappers delegan en TIM_InitTimer/Enable/ReadTimer; encabezado
sin CMSIS, lectura ms, configuración coherente, compilación y regresiones vigentes.
Funcionamiento físico requiere comprobación posterior de reloj/placa.

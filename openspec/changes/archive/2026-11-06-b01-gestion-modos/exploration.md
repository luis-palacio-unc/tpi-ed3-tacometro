# Exploración B01 — 2026-11-06

B00 resolvió el build; Procesamiento y los servicios siguen como esqueletos.
R05/R06 exigen exclusión de mandos y nueva consigna al entrar en automático.
Las responsabilidades están en Procesamiento.h y Mandos/Comunicacion/Driver_Motor.h;
las interfaces públicas aún no están definidas.

Se consideró seguir con pines/periféricos, pero faltan conexiones y revisión física.
La lógica de modos permite ejecutar pruebas C en host y comprobar primero una regla
central del sistema, manteniendo pendientes sus pruebas eléctricas.

Se recomienda una lógica aislable de hardware dentro de Procesamiento, sin introducir
otra capa ni implementar un controlador simulado. En automático activo sólo se
habilitará la ruta del controlador futuro. El cálculo de PWM automático queda pendiente.

Riesgo principal: confundir una consigna procesada después del cambio con una recibida
después. La interfaz deberá preservar antigüedad; especificar orden para eventos
simultáneos antes de implementar. No se eligieron rangos físicos, pines ni algoritmo PI/PID.

Listo para propuesta. No se escribieron especificación contractual, diseño o código B01.

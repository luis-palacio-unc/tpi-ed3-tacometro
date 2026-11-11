# Exploración B15 — período obtenido de MR0

Base actual c952892, main. El usuario autoriza eliminar el parámetro período
y leer directamente el registro en Motor.c; conserva PWM1.1/P2.0 y escritura
por driver. Es una corrección del cambio B15 activo, aún sin aceptar/archivar.

Biblioteca original revisada en sólo lectura: lpc17xx_pwm.h ofrece actualización
de match pero ningún getter de MR0; PWM_GetCaptureValue devuelve captura, no
período. LPC17xx.h define MR0 __IO uint32_t (volatile) dentro de LPC_PWM_TypeDef
y LPC_PWM1 como puntero al periférico. Leerlo aquí corresponde a Dispositivos.
No modificar Drivers, crear getter de biblioteca o pasar configuración duplicada.

La lectura puede ver valor programado aún no latcheado; exigir configuración
efectiva/estable sin update pendiente y acceso serializado. No afirmar que una
lectura resuelva concurrencia o confirme electricidad. Inicialización al final.
Fundamento previo de pin/extremos/latch en UM10360 tabla84/p120 y cap24 se conserva;
no se incorporan nuevos requisitos desde documentos externos.

Pruebas host redirigen sólo el puntero del periférico a RAM con el tipo CMSIS
original, mediante preinclude privado de pruebas_motor. Compilación ARM y
firmware usan puntero original. No introducir hooks o abstracciones en producción.
Decisión anterior con período explícito y su evidencia permanecen en Git c952892;
informe actual verifica exclusivamente código corregido.

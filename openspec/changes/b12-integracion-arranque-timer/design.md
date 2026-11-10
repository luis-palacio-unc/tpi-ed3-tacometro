# Diseño preliminar B12

Ruta prevista: Reset copia .data y borra .bss; actualiza la información RAM del
reloj con la función CMSIS original; entra a main; main llama los wrappers Init y
Start una vez y mantiene el lazo. Aplicación/servicios no incluyen tipos ni registros
LPC. No llamar SystemInit ni agregar configuración activa de PLL/flash.

Vincular sólo fuentes requeridas TIM/CLKPWR/SystemCoreClock y aserciones; bibliotecas
originales, sin forks/copias parcheadas. Casos de nombres de includes y diagnósticos
se resuelven en build/soporte externo conservando controles. Detalle de integración
a cerrar durante B12 si se aprueba; no silenciar fallos de compilación.

Firmware normal debe retener init/start y reloj; fixture separado también llama Read
para comprobar enlace real de las tres operaciones en Debug/Release, sin proveedores
falsos. Verificación de .data/.bss, vectores, CRP y límites conserva controles B00.
Extensión de startup se documentará; B00-R3 candidato incluye actualización sólo RAM.

TIMER0 exclusivo desde reset, sin ISR/match/captura. PCLK/período representables y
SystemCoreClock coherente son precondiciones; mantener macros CMSIS originales,
confirmar fuente física si se usa oscilador externo. No diseñar selector sin GPIO
ni inyectar entradas ficticias en producción para demostrar tiempo.

Verificación: regresiones host, ARM Debug/Release y fixture, inspección de orden y
símbolos, Doxygen, hashes completos de Drivers. Plan de prueba física separado;
ejecutar/flashear requiere alcance autorizado. Nuevas/modificadas USW documentadas.

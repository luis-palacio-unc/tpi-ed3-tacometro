# Diseño B11 — Timer directo

Dispositivos/Timer.c incluye lpc17xx_timer.h y fija LPC_TIM0. Timer.h sólo declara
las tres funciones y TIMER_BASE_MS (1 por defecto, override de build), uint32_t.
Init construye TIM_TIMERCFG_T con TIM_US y base_ms*1000; Start llama TIM_Enable;
Read multiplica TIM_ReadTimer por base_ms. Sin estado mutable, callbacks ni servicios nuevos.
Configuracion/Temporizacion.h se borra. Init no arranca por sí mismo.

El driver calcula PR y usa SystemCoreClock/PCLK; exige reloj correcto y período
representable/exacto. TIMER0 exclusivo desde reset sin match/captura previo.
APIs serializadas; reinit crea época nueva, consumidores deben renovar marcas.
Solo configuración fija por compilación; no agregar errores/estados ajenos a la API.
MS uint32 modular, sin acumulación ni reinicio por lectura; intervalos<2^31ms.

ESW.2.1.17 conserva USW.17.1 init y .17.2 lectura; nueva .17.14 arranque para
no reutilizar IDs de candidatos retirados. Documentación Doxygen incremental.
Build host produce objetos; no ejecutar llamadas de periférico en host. ARM
compila contra headers originales. Main sigue mínimo, integración de arranque
y enlace del conjunto de drivers usados corresponde a la integración física.
Verificar llamadas/símbolos del objeto, regresiones y Doxygen; no inventar simulación.
No crear pruebas que sólo repitan las llamadas del wrapper.

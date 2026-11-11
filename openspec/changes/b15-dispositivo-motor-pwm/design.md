# Diseño candidato B15 — Motor PWM

## Enfoque y decisiones

Motor es esqueleto ESW.2.1.9; Driver_Motor B08 recibe mando por mil mediante
callback bool(void*,uint16_t). Su adaptación queda para integración final y no
se cambia ese contrato aceptado. El dispositivo recibe mando y período explícito.

| Elección | Razón y alternativa |
|---|---|
| PWM1.1/P2.0 función 01, single-edge | Disponible en biblioteca/manual; evita P1.18 asignado a CAP1.0. No usar TIMER0/1 para PWM. |
| Motor_Write(mando_por_mil, periodo_ticks) | Período explícito permite convertir sin globales ni leer MR0 por registros; configuración coherente externa. No fijar reloj/frecuencia todavía. |
| bool sólo por validación real | false rechaza dominio inválido; true despacha al driver void sin inventar diagnóstico eléctrico. Sin contexto no usado. |
| PWM_MatchUpdateSingle(PWM_MATCH_1, valor) | Biblioteca existente actualiza MR1/LER; no duplicar acceso a registros ni modificar Drivers. |
| Conversión local, servicio conservado | Dispositivos conoce ticks/match; normalización/decisión del servicio sigue portable. Adaptador B08 se definirá al integrar. |

## Conversión y extremos

Validar mando <=1000 y 1<=período<UINT32_MAX antes del despacho. Constante de
mando máximo compartida de Configuracion/Limites.h, sin nueva fuente divergente.
Calcular q=round_half_up(mando*período/1000) con intermedio uint64_t: producto
máximo 1000*(2^32-2), más 500, cabe en 64 bits. Resultado q entre 0 y período.
Enviar MR1=q si q<período; si q=período, enviar período+1 para alto continuo.
Así no se depende de prioridad entre matches simultáneos MR0/MR1. Rechazar
período UINT32_MAX deja espacio para ese centinela. Período 1 admite sólo 0/100 %;
cuantización esperada, no pérdida intermedia ni imposición de frecuencia.

Manual UM10360 §24.4.1/p524: match cero deja salida baja; match mayor que MR0
evita flanco de bajada, salida alta continua. §24.6.7/p531: MR/LER se aplican en
el próximo límite de ciclo. El wrapper no espera ni garantiza que órdenes rápidas
se observen todas físicamente; puede prevalecer la última antes del latch.

## Precondiciones y flujo

Integración final seleccionará PWM1_P2_0 con PWM_PinConfig, configurará prescaler,
MR0, canal/salida/contador y arranque usando biblioteca. El período suministrado
debe coincidir con MR0 efectivo y estable, sin actualización de MR0 pendiente;
PWM exclusivo, reset por MR0, sin otros matches que alteren TC y acceso serializado.
La frecuencia depende de reloj/prescaler/MR0 y no se elige en este bloque.
La orden 0 % no acredita detención/freno del eje ni define transitorios de arranque.

Flujo B15: mando/período → validar/convertir en Motor_Write → driver de match.
El dispositivo no asigna objetos de estado ni incorpora callbacks/proveedores nuevos.

## Archivos y verificación prevista

- Dispositivos/Motor.h/.c: una USW pública documentada, código y validación mínimos.
- tests/host/test_motor.c: doble únicamente de PWM_MatchUpdateSingle, sin ejecutar
  biblioteca/MMIO. Casos S1–S4 y expectativas por fracciones/racionales independientes:
  extremos, cuantización, empates, repeticiones, errores/recuperación y períodos grandes.
- CMakeLists.txt: target/test host estricto/sanitizadores; conservar 18 objetos base.
- docs/doxygen/Doxyfile y README/docs/agente/SDD: ampliar INPUT y trazabilidad.

Host/sanitizadores/regresiones, compilación ARM O0/O2/header aislado y símbolos/
recursos propios, Doxygen estricto y diff/hashes. Comprobaciones numéricas no
acreditan frecuencia, duty eléctrico o movimiento. Sin init/global link/main.
Pendientes para integración: frecuencia, etapa de potencia/polaridad/cableado,
compatibilidad eléctrica, adaptación bool/contexto B08 y ensayo real 0/50/100 %.
Implementar sólo tras acuerdo, sin cambios en Drivers ni contratos aceptados.


Alcance/PWM1.1/P2.0 acordados2026-11-10, implementado con el diseño propuesto.
Verificación real y límites en verify-report.md; resultado pendiente de aceptación.
No sincronizar contratos ni archivar hasta aceptar el resultado.

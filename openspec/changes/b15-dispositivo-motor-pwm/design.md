# Diseño B15 — envío directo con MR0=1000

## Decisión actual

El usuario fija MR0=1000 y solicita pasar mando_por_mil directamente al driver.
Esto reemplaza período explícito/lectura y conversión de c952892/cf17359.
Se conserva bool Motor_Write(uint16_t mando_por_mil), PWM1.1/P2.0 y el máximo
compartido de Limites.h. El servicio sigue portable e intacto.

| Elección | Razón |
|---|---|
| Validar mando y llamar PWM_MatchUpdateSingle(PWM_MATCH_1,mando_por_mil) | Período fijo1000 coincide con la escala del mando, sin conversión. |
| Sin lectura/escritura de MR0 en Motor_Write | MR0 se configura una vez al integrar; no acoplar actualización de duty al arranque. |
| No ajustar1000 a1001 | Orden explícita actual; fundamento/limitación física PWM.1 en exploration.md. |
| Retirar modelo PWM en RAM y preinclude host | Sin lectura de registros, basta el doble de la función de escritura. |

## Contrato y recursos

Mando>1000 devuelve false sin llamadas ni cero automático. Mando válido llama
una vez al driver y devuelve true, incluidas repeticiones. Sin saturación185,
caché/contexto/estado/aritmética/64bits/init/heap/ISR/esperas. true no diagnostica
hardware. Cada solicitud sustituye la anterior; latcheo físico al próximo ciclo.

La inicialización final configurará MR0=1000 con la biblioteca existente y el
reloj/prescaler para la frecuencia que se acuerde. Este bloque deja esa condición
escrita; no inicializa ahora ni escribe MR0 por llamada. PWM exclusivo/configurado/
arrancado, single-edge alto, MR0 efectivo/estable y reset por MR0, sin otros matches
que alteren TC y acceso serializado. Ver limitación física PWM.1 en exploración.

## Archivos y verificación

- Motor.h/.c: USW9.1 con una entrada, validación y despacho literal, Doxygen actualizado.
- tests/host/test_motor.c: cuatro casos de referencias/dominio completo/rechazo y
  recuperación/repetición-cero; 1001 valores válidos y64535 inválidos.
- CMakeLists.txt: retirar sólo preinclude de pruebas_motor; conservar target/flags/tests.
- Eliminar tests/host/dobles/pwm_registros.h, propio del test anterior sin uso actual.
- README/docs-agente/docs-doxygen/SDD: estado y evidencia de contrato actual.

Ejecutar B15 y regresiones host/san, ARM O0/O2/header aislado, Doxygen estricto,
símbolos/recursos/diff/hashes. Esperar único símbolo externo PWM_MatchUpdateSingle,
sin helpers aritméticos ni puntero a modelo/registros. Dobles registran órdenes,
no simulan MCU/tiempo/latch/electricidad ni comprueban errata física.
No tocar biblioteca o módulos aceptados, integrar main, configurar periféricos,
instalar/flashear/publicar o avanzar bloques. Commit local al terminar; resultado
pendiente de aceptación antes de sincronizar/archivar.

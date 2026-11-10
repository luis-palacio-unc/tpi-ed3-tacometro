# Estado — 2026-11-10

## Punto actual

- Repositorio `/home/luis/Desktop/Universidad/ed3/tpi/Trabajo Practico Integrador EDIII`;
  rama main, consultar HEAD y origin/main. Base aceptada c543fee; fetch previo sin
  divergencia. Usuario autorizó este cierre y push de pendientes.
- B00–B11/B03-R1 aceptados/archivados. B11: tres wrappers, contrato vigente timer;
  archivo2026-11-10-b11-wrapper-timer. Antecedentes B11 retirados no aceptados.
- B12 `b12-integracion-arranque-timer` sólo propuesto, scope_approved=false.
  No implementar hasta acuerdo; no iniciar bloques posteriores.
- Drivers originales restituidos por usuario:90 archivos coinciden con a21cc1e.
  Prohibido modificarlos. B11 conserva sólo Init/Start/Read, base1ms configurable
  en Timer.h y control #if/#error; no Temporizacion.h/Tiempo/proveedor/puerto/ISR nueva.
- Cierre/propuesta documentales; commit normal autorizado, push solicitado sin force,
  conservar hooks/firma y verificar HEAD==origin/main==remoto. No merge autorizado.

## Evidencia y límites

Aceptación B11 conserva evidencia histórica: host17/17 (179 C + oráculo Python),
ARM13/13 compilación/controles, Doxygen35, header C11/C++17/ARM-C11 y límites de base.
Drivers90 hashes originales intactos. Timer no fue ejecutado en host/ARM/placa.
El wrapper sin uso se descarta por GC: no se enlazó el conjunto de biblioteca ni se
integró arranque. Sincronizar/archivar no acredita precisión física ni main integrado.
Informe B11 archivado. Cierre actual no repite ni declara nuevas pruebas de firmware.
Exploración B12: sintaxis CMSIS original ARM-C11 PASS, no prueba de reloj/arranque.
Logs/manifiestos/publicación externos: /tmp/ed3-cierre-b11-propuesta-b12-yiwhqz1e.

## Próxima acción

Acordar B12: enlazar originales, actualizar información SystemCoreClock vía CMSIS
tras .data/.bss, Init/Start desde main; fixture de consulta y controles Debug/Release.
API B11/capas/biblioteca preservadas, sin SystemInit/reconfigurar PLL/flash.
Compatibilidad de nombres de headers/diagnósticos se resuelve fuera de biblioteca.
GPIO/selector físico, ventanaRPM/ADC/UART/PWM/control y conexión de consumidores
pendientes de otros bloques. Frecuencia/precisión física/PCB/silicio/pines/VREF y
ensayo de timer en placa pendientes; no flashear/instalar sin alcance autorizado.

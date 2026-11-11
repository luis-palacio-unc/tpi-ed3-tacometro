# Exploración B15 — período fijo1000

Base cf17359. Usuario ordena MR0=1000 y envío literal del mando. Se retiran
lectura/conversión/cálculo de match, manteniendo validación0..1000 y driver original.
La inicialización de periféricos sigue para el final; su contrato queda MR0=1000.

Fuente de discrepancia del100 %: UM10360 §24.4.1 asegura salida alta con MR1>MR0,
pero no implica que igualdad sea inválida. La explicación previa que exigía1001
fue demasiado restrictiva. [NXP ES_LPC176x rev10.6, §3.13/p17](https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf)
identifica MR1=MR0 como100 % y MR0+1 como workaround de PWM.1. La orden actual
exige envío directo; no se introduce ese workaround ni cambia canal sin acuerdo.
PWM.1 queda como limitación física pendiente de revisar al integrar PWM1.1.

La biblioteca PWM_MatchUpdateSingle ya escribe match/LER; no modificarla.
Retirar modelo RAM/preinclude porque sólo queda escritura por API. Probar el
valor exacto enviado (incluido1000) y rechazo/recuperación; el doble no valida
señal eléctrica ni errata. Historial anterior/evidencia permanece en Git.

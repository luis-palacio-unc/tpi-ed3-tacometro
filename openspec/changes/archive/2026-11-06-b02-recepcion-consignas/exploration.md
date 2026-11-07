# Exploración B02

Comunicacion sigue como esqueleto y Procesamiento ya dispone de contrato aceptado.
La aplicación necesita RPM válidas y marca de recepción conservada, no la fecha de
extracción de una cola. El máximo acordado es 185 RPM.

Se consideró avanzar con temporizadores/PWM, pero faltan pines y configuración
física. El protocolo de consignas permite comprobar el origen de los eventos y su
relación con los modos mediante C host, sin afirmar comunicación eléctrica.

Recomendación: parser incremental y eventos acotados en la capa de servicios. El
servicio no debe incluir Procesamiento.h; un adaptador de aplicación/test convierte
sus eventos al contrato B01. Resolver la fuente común/configuración del máximo sin
romper capas; no crear abstracciones ajenas a este requisito.

B02 sólo propone gramática/alcance. No hay código, contrato vigente, pruebas ejecutadas
ni acuerdo del bloque todavía. UART física y telemetría se planificarán aparte.

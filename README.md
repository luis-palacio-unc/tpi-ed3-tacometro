# TPI EDIII — Tacómetro digital

Firmware C para LPC1769/OM13085, CLion y SDD por bloques.
Leer primero [ESTADO](docs/agente/ESTADO.md). Ver también
[CONTEXTO](docs/agente/CONTEXTO.md), [ORQUESTADOR](docs/agente/ORQUESTADOR.md),
[MAPA_REQUISITOS](docs/agente/MAPA_REQUISITOS.md) y
[verificación B00](openspec/changes/archive/2026-11-06-b00-build-lpc1769/verify-report.md).

## Compilar en CLion o terminal

Desde la raíz del proyecto, usar el preset CMake `lpc1769`:

```sh
cmake --preset lpc1769
cmake --build --preset lpc1769
ctest --preset lpc1769
```

Requisitos ya disponibles en el entorno de diagnóstico: CMake >=3.20, GCC ARM,
GNU binutils ARM, Python >=3.8 estándar y compilador C host para la prueba de objetos
host. No se incorporan paquetes Python, libc ni frameworks externos.
Si las herramientas ARM no están en PATH, indicar el directorio bin con
`-DARM_TOOLCHAIN_DIR=/ruta/al/bin` al configurar. Un directorio inválido falla.

Salidas: `build/lpc1769/firmware.elf`, `firmware.map` y `memoria.txt`.
El build ejecuta automáticamente el verificador del ELF; CTest agrega casos negativos.
`firmware.elf` es para el MCU: no se ejecuta como programa de escritorio.
No está configurada todavía una sesión de depuración/flasheo de CLion.

Perfil host: compila objetos y ejecuta pruebas C de Procesamiento, Comunicacion, Medicion_Velocidad, Mandos, Temperatura y LM35, incluida su integración, sin ejecutar main del firmware:

```sh
cmake --preset host-objects
cmake --build --preset host-objects
ctest --preset host-objects
```

Para mantener todos los resultados fuera del proyecto también se admite:

```sh
cmake -S . -B /tmp/ed3-arm -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"
cmake --build /tmp/ed3-arm
ctest --test-dir /tmp/ed3-arm --output-on-failure
```

## Alcance actual

B00 implementa build ARM, vectores, linker y main en bucle. El startup copia .data,
borra .bss y llama main; las IRQ sin implementación quedan en un handler por defecto.
No configura reloj/PLL ni periféricos, ni llama SystemInit heredado. No supone una
frecuencia de reloj al entrar desde el bootloader. Pila reservada de 2 kB, sin heap;
las dos SRAM AHB quedan sin asignar. La imagen reserva CRP con valor 0xffffffff.

La verificación estática y las pruebas de herramientas no acreditan arranque físico,
tiempos ni funcionamiento eléctrico. No se ejecutaron firmware ni pruebas heredadas
en placa. La revisión física de PCB/silicio permanece pendiente.

Se preservan `Aplicacion/`, `Servicios/`, `Dispositivos/` y `Drivers/`.
La biblioteca de Drivers no se compila completa en B00; se verifican sus encabezados
CMSIS. Los demás módulos del tacómetro siguen pendientes de implementación.

Repositorio privado: https://github.com/luis-palacio-unc/tpi-ed3-tacometro
B00 aceptado y archivado. Publicación inicial en main autorizada por el usuario;
no extiende autorización a bloques futuros. [Contrato build vigente](openspec/specs/build/spec.md).

B01 aceptado/archivado con límite 185 RPM. [Contrato de modos vigente](openspec/specs/modos/spec.md).
B02 aceptado y archivado. [Contrato de comunicación vigente](openspec/specs/comunicacion/spec.md).
B03 aceptado y archivado. [Contrato de medición vigente](openspec/specs/medicion-velocidad/spec.md).
B04 aceptado y archivado. [Contrato de mandos vigente](openspec/specs/mandos/spec.md).
B05 corregido, aceptado y archivado. [Contrato de temperatura vigente](openspec/specs/temperatura/spec.md)
y [evidencia](openspec/changes/archive/2026-11-07-b05-conversion-temperatura/verify-report.md).

## B01 y documentación incremental

Procesamiento.h/.c implementan selección de modo/fuente, límite 0..185 RPM y validez temporal de consignas,
sin hardware ni regulación PI/PID. main sigue mínimo. Contratos, escenarios y evidencia
están en openspec/changes/archive/2026-11-06-b01-gestion-modos/.

Para instrumentar pruebas host (en un build independiente):

```sh
cmake -S . -B /tmp/ed3-host-san -DTPI_BUILD_ARM=OFF -DTPI_HOST_SANITIZERS=ON
cmake --build /tmp/ed3-host-san
ctest --test-dir /tmp/ed3-host-san --output-on-failure
```

Doxygen ya disponible en este entorno; no es requisito para compilar el firmware.
Desde la raíz, `doxygen docs/doxygen/Doxyfile` genera HTML/XML parciales en
build/doxygen/. Ver [convención documental](docs/doxygen/README.md). Estos resultados
no constituyen aún la documentación final de todos los módulos.

## B02: recepción lógica de consignas

Enviar ASCII `RPM 185` terminado en LF o CRLF; se admiten 1..3 dígitos, ceros
iniciales y valores 0..185. Las tramas inválidas se rechazan, sin saturar valores.
Comunicacion procesa bytes y entrega eventos FIFO con la marca del LF recibido;
no accede todavía a la UART real ni envía telemetría. Cada llamada recibe un orden
asignado al recibir el byte, creciente y serializado externamente con los cambios
de modo. Buffers de trama/cola acotados; errores explícitos y recuperación.

El máximo común vive en Configuracion/Limites.h. Las USW de Comunicacion y la
constante están incluidas en el Doxyfile parcial. Ver
[especificación vigente](openspec/specs/comunicacion/spec.md)
y [verificación B02](openspec/changes/archive/2026-11-06-b02-recepcion-consignas/verify-report.md).
Con sanitizadores, ejecutar fuera de depurador/ptrace para que LeakSanitizer funcione.

## B03: cálculo lógico de velocidad

B03 original y revisión B03-R1 están aceptados/archivados. Revisión aceptada
el 2026-11-09; contrato vigente sincronizado. La API actual recibe pulsos, duración_us y
pulsos_por_vuelta uint32_t explícitos; no conoce el disco ni incluye su configuración.
El TPI mantiene 60 pulsos/vuelta en Configuracion/Medicion.h para un flanco/ranura.
Cambiar factor no requiere recompilar el servicio ni cambiar unidades para aplicación.

Salida: milésimas de RPM y estado. Cero pulsos con duración/factor positivos es cero
válido. Cercano/empate arriba, sin redondeo intermedio y error aritmético <=0,0005 RPM.
Prioridad de errores: NULL, duración cero, factor cero y resultado fuera de uint32_t.
Error escribible invalida y limpia; siguiente ventana válida recupera. No recorta
mediciones a 185 RPM. Captura/configuración coherentes y acceso serializado externos;
sin captura física, timeout ni main integrado.

23 casos C B03 y oráculo Python estándar de 11199 ternas pasan en host/sanitizadores.
Regresiones: 102 casos C totales; ARM Debug 8/8 y fixture B03 Release; Doxygen estricto.
Servicio y consumidor aislados sin configuración/dispositivo. Cálculo uint64_t
acotado y libgcc existente, sin libc en firmware. Compilar/enlazar no acredita
ejecución física. Ver [verificación B03-R1 aceptada](openspec/changes/archive/2026-11-09-b03-factor-pulsos-configurable/verify-report.md)
y [evidencia B03 original](openspec/changes/archive/2026-11-06-b03-medicion-velocidad/verify-report.md).

## B04: normalización del mando manual

Mandos.h/.c convierte lectura y fondo de escala explícitos (uint32_t) en mando
0..1000 por mil (uint16_t), independiente de bits, VREF o canal del ADC. Ejemplo:
2048/4095 produce 500, equivalente al 50 % del mando lógico. Redondeo cercano,
empate arriba; extremos 0 y fondo de escala producen exactamente 0 y 1000.
Inicializar deja SIN_DATOS. Escala cero o lectura excesiva invalidan y borran el
valor anterior; sólo VALIDO permite consumo. No implementa selector, ADC/DMA o PWM.

El límite manual tiene fuente común en Configuracion/Limites.h; Procesamiento
conserva su alias y comportamiento. Prueba/fixture adapta salidas válidas a B01;
no hay dependencia del servicio hacia aplicación ni integración del main real.
CTest agrega 15 casos B04 con recorridos, escalas equivalentes, errores y fronteras;
Doxygen documenta sus dos USW. Ver [informe B04](openspec/changes/archive/2026-11-06-b04-normalizacion-mando-manual/verify-report.md).

## B05: temperatura genérica y dispositivo LM35

Servicios/Temperatura.h/.c recibe temperatura ya expresada en centésimas Celsius
(int32_t) y estado de fuente válido/sin datos/error. Preserva valores válidos o
invalida con cero; diferencia cero válido de ausencia. No conoce el sensor concreto.
Dispositivos/LM35.h/.c contiene la conversión ADC→Celsius y su ganancia nominal,
con escala/referencia explícitas y errores de conversión. Todavía no adquiere muestras.

El servicio compila y enlaza por separado, sin LM35; las pruebas adaptan datos del
dispositivo al contrato genérico y conservan casos numéricos/fronteras de B05.
La composición con adquisición real se resolverá después, sin acceso directo a
dispositivos desde lógica de aplicación. ADC/DMA, alarma y main siguen pendientes.
Ver [informe corregido](openspec/changes/archive/2026-11-07-b05-conversion-temperatura/verify-report.md);
[informe inicial](openspec/changes/archive/2026-11-07-b05-conversion-temperatura/verify-report-inicial.md)
conserva la evidencia de la arquitectura anterior y no acredita la API actual.

## Próximo bloque propuesto

[B06: evaluación lógica de alarma por temperatura](openspec/changes/b06-logica-alarma-temperatura/proposal.md).
Umbral como parámetro de configuración confirmado por usuario; sin valor fijo.
Alcance pendiente de acuerdo; no hay implementación ni accionamiento físico nuevo.

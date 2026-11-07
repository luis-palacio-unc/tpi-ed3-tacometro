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

Perfil host: compila objetos y ejecuta pruebas C de Procesamiento y Comunicacion, sin ejecutar main del firmware:

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
Bloque actual: [B02 — recepción de consignas](openspec/changes/b02-recepcion-consignas/proposal.md),
implementado/verificado; resultado pendiente de aceptación.

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
[especificación candidata](openspec/changes/b02-recepcion-consignas/specs/comunicacion/spec.md)
y [verificación B02](openspec/changes/b02-recepcion-consignas/verify-report.md).
Con sanitizadores, ejecutar fuera de depurador/ptrace para que LeakSanitizer funcione.

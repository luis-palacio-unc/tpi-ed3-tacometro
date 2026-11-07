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

Perfil host, limitado a compilar objetos sin ejecutar main:

```sh
cmake --preset host-objects
cmake --build --preset host-objects
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
CMSIS. El resto de módulos del tacómetro sigue pendiente de implementación.

Repositorio privado: https://github.com/luis-palacio-unc/tpi-ed3-tacometro
B00 aceptado y archivado. Publicación inicial en main autorizada por el usuario;
no extiende autorización a bloques futuros. [Contrato build vigente](openspec/specs/build/spec.md).

Siguiente bloque: [B01 — gestión de modos](openspec/changes/b01-gestion-modos/proposal.md),
propuesto y sin implementación.

# Diseño B00

## Enfoque y decisiones

| Decisión | Alternativa | Motivo |
|---|---|---|
| CMake con opción TPI_BUILD_ARM y presets separados | Detectar ARM automáticamente | Evitar compilar host por error para placa |
| Toolchain con ARM_TOOLCHAIN_DIR opcional y búsqueda explícita de gcc/binutils | Instalar SDK/framework | Reutilizar herramientas existentes |
| Startup GNU assembler, tabla de 51 palabras y handlers débiles | Startup C con casts de punteros | Pila y vectores explícitos sin extensiones de inicializadores C |
| SRAM local 32 kB; AHB0/AHB1 declarados, sin uso en B00 | RAM contigua de 64 kB | Respetar bancos del LPC1769 |
| Reserva de pila 2 kB, sin heap | Introducir malloc/libc | Arranque mínimo; reserva no prueba profundidad real |
| Checksum por linker; palabra CRP 0xffffffff en 0x2fc | Postprocesar binario o permitir texto en CRP | Validar imagen original y prevenir protección accidental |
| No llamar SystemInit heredado | Adoptar PLL/flash de la biblioteca | Fuera de alcance, requiere revisión de placa/reloj |
| Verificador Python estándar que consulta binutils | Añadir pyelftools | Evitar dependencias; validar ELF y salida de herramientas |

## Mapa y arranque

Flash 0x00000000/512 kB; SRAM local 0x10000000/32 kB;
AHB0 0x2007c000/16 kB; AHB1 0x20080000/16 kB.
Vectores en 0, checksum en palabra 7. CRP en 0x2fc, texto desde 0x300.
.data: VMA SRAM y LMA flash; .bss NOLOAD; stack NOLOAD al final de SRAM.
ASSERT comprueba espacio, alineación y tabla. No hay heap, IAP ni bootloader propio.

Reset_Handler copia palabras alineadas entre símbolos __data_load__/__data_start__/
__data_end__, pone a cero __bss_start__/__bss_end__, llama main y queda detenido si
main retorna. Excepciones/IRQ no implementadas van a Default_Handler en bucle.
main es un bucle sin dispositivos ni reloj. El estado de reloj heredado no se
supone: el boot ROM puede dejar PLL activo tras ISP. B00 no mide SystemCoreClock.

## Archivos e interfaces

- CMakeLists.txt, CMakePresets.json, cmake/arm-none-eabi.cmake: perfiles/build.
- plataforma/lpc1769.ld, startup_lpc1769.S, cmsis_compat.c: mapa/arranque y
  compatibilidad de IRQn de los encabezados existentes.
- Aplicacion/main.c: entrada mínima, documentación ESW y USW.
- scripts/verificar_elf.py: CLI `ELF --tool-prefix PREFIX [--report FILE]`;
  código 0 sólo tras todas las comprobaciones, 1 ante error.
- scripts/pruebas_b00.py: integración en CTest; fixtures temporales compilados,
  corrupción de imágenes y configuración fallida; sin ejecución en MCU.
- README y docs/agente: comandos y alcance real de la evidencia.

No migración ni cambios en Drivers. El perfil host conserva el target firmware
como biblioteca de objetos; el perfil ARM produce firmware.elf, firmware.map y memoria.txt.
Warnings estrictos para código propio; includes CMSIS como SYSTEM.
Objetos ARM declaran noexecstack con -Wa,--noexecstack; el linker conserva --fatal-warnings.
Extracción con objcopy a salida temporal explícita, sin reescribir ELF original.
Los atributos se comparan por arquitectura/perfil/Thumb, no por nombre descriptivo CPU.

## Verificación

S1/S3: build real e inspección del ELF; S2: configuración sin toolchain y con ruta
inexistente; S4: imágenes alteradas; S5: linker con exceso de SRAM/main ausente;
S6: fixture .data/.bss, símbolos y desensamblado del startup, más revisión manual;
S7: build host en directorio independiente. Casos negativos deben fallar por el motivo
esperado. No ejecutar las pruebas heredadas de periféricos.

## Referencias y pendientes

[NXP UM10360 rev.2, copia Keil](https://www.keil.com/dd/docs/datashts/philips/lpc17xx_um.pdf):
cap.2 mapa; cap.32 checksum/CRP. Rev.3.1 figura en NXP pero requiere cuenta.
[Datasheet rev.9.10](https://www.nxp.com/docs/en/data-sheet/LPC1769_68_67_66_65_64_63.pdf)
y erratas rev.10.6 complementan identificación. Pendientes físicos: revisión
PCB/silicio y validación de arranque, fuera de la aceptación estática de B00.

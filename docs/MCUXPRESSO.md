# Proyecto MCUXpresso (I3)

La raíz del repositorio es un proyecto de MCUXpresso IDE (`.project` y `.cproject`),
llamado `TPI_ED3_Tacometro`, para LPC1769. Se importa sin copiar archivos: hay un solo
árbol de fuentes, compartido con el build CMake. Preparado y compilado sin interfaz
gráfica con MCUXpresso IDE 25.6.136 en Linux. **La importación por la interfaz, la
conexión con la sonda y la depuración no se probaron: quedan para la puesta en marcha.**

## Importar y compilar

1. `File → Import… → General → Existing Projects into Workspace`.
2. `Select root directory`: la raíz del repositorio (o la carpeta `Firmware/` de la
   entrega). Dejar **sin marcar** `Copy projects into workspace`. `Finish`.
3. Elegir la configuración con el martillo (`Debug` o `Release`) y compilar
   (`Project → Build Project`). El resultado es `Debug/TPI_ED3_Tacometro.axf`.

Compilación sin interfaz, sobre una copia para no generar archivos en el repositorio:

```sh
/usr/local/mcuxpressoide-25.6.136/ide/mcuxpressoide -nosplash --launcher.suppressErrors \
  -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
  -data <workspace-temporal> -import <copia-del-repo> -cleanBuild all
python3 scripts/verificar_elf_mcuxpresso.py <copia>/Debug/TPI_ED3_Tacometro.axf \
  --tool-prefix /usr/local/mcuxpressoide-25.6.136/ide/tools/bin/arm-none-eabi-
```

La IDE genera `Debug/`, `Release/`, `.settings/` y, al depurar, archivos `*.launch`.
Están en `.gitignore`: dependen de la máquina y se regeneran solos.

## Cómo está configurado

| Tema | Valor |
|---|---|
| MCU | LPC1769, Cortex-M3; flash 512 kB, RAM local 32 kB y AHB 32 kB |
| Fuentes | `Aplicacion`, `Servicios`, `Dispositivos`, `plataforma` y `Drivers` (los mismos 10 `.c` de la biblioteca que usa CMake) |
| Excluido | `plataforma/startup_lpc1769.S`, `plataforma/lpc1769.ld`, `core_cm3.c`, los drivers sin uso, `tests`, `scripts`, `cmake`, `docs`, `openspec`, `build` |
| Arranque | `plataforma/mcuxpresso/cr_startup_lpc175x_6x.c` y `crp.c`, copiados sin cambios del asistente de la IDE |
| Símbolos | `__NEWLIB__`, `__CODE_RED`, `CORE_M3`, `__LPC17XX__`; `NDEBUG` sólo en Release |
| Inclusión | `Aplicacion`, `Servicios`, `Dispositivos`, `Dispositivos/soporte`, `Configuracion`, `Drivers/CMSISv2p00_LPC17xx/inc`, `Drivers/CMSISv2p00_LPC17xx/Drivers/inc`, `plataforma/compat` (relativas al proyecto) |
| Lenguaje | C11 estricto (`-std=c11`); Debug `-O0 -g3`, Release `-Os -g` |
| Avisos | Código propio: `-Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes`. `Drivers` y `plataforma/mcuxpresso`: sólo `-Wall` |
| Biblioteca C | NewlibNano sin E/S (`none`); no se usa `printf` ni memoria dinámica |
| Enlace | Script administrado por la IDE, `--gc-sections`, CRP habilitado con `CRP_NO_CRP` |

Tres cosas que no hay que cambiar:

- **No definir `__USE_CMSIS` ni `__USE_LPCOPEN`.** Con ellos `ResetISR` llama a
  `SystemInit` y el PLL se configuraría dos veces, la primera sin ajustar antes los
  tiempos de la flash. El reloj lo configura `Reloj_Init`, primera llamada de `main`.
- **No definir `DEBUG`.** Activa las comprobaciones de parámetros de la biblioteca
  (`CHECK_PARAM`), que el build CMake no usa.
- **No usar Redlib.** Su `stdint.h` define `UINT32_C` con una conversión de tipo y las
  comprobaciones `#if` de `Timer.h`, `UART.h` y `Analogica.h` dejan de compilar.

Las carpetas de la biblioteca también figuran con `-isystem` en *Miscellaneous → Other
flags*: así sus encabezados no generan avisos con las opciones estrictas, igual que en
CMake.

## Depurar con la sonda de la placa

1. Conectar por USB el lado de la sonda de la LPCXpresso LPC1769 (CMSIS-DAP).
2. Seleccionar el proyecto y pulsar `Debug` en el panel *Quickstart* (escarabajo azul).
3. La IDE busca sondas; elegir la **LinkServer (CMSIS-DAP)** y aceptar. La primera vez
   crea sola `TPI_ED3_Tacometro LinkServer Debug.launch`; no hay que configurarlo.
4. Programa la flash y se detiene al entrar en `main`. `Resume` (F8) para ejecutar.

Variables y registros útiles:

- `SystemCoreClock` (vista *Global Variables* o *Expressions*): debe valer 100000000
  después de `Sistema_Init`.
- `Aplicacion_Estado` (estática de `main.c`): 0 MANUAL, 1 AUTO_ESPERA, 2 AUTO_ACTIVO.
- `Aplicacion_Regulador`: memoria del PI.
- Vista *Peripherals+*: `PWM` (MR0 = 1000, MR1 = mando), `TIMER0`, `TIMER1` (TC = pulsos),
  `UART0`, `ADC`, `GPIO0` (P0.22 LED) y `GPIO2` (P2.6 llave, P2.7 buzzer).

Usar la configuración `Debug` para depurar; en `Release` el optimizador elimina variables.

### Si la IDE no encuentra la sonda

- Revisar el cable (algunos sólo cargan) y que esté en el conector de la sonda.
- `lsusb | grep 1fc9`: `1fc9:0090` es la sonda en modo CMSIS-DAP; `1fc9:000c` es modo
  DFU, y la IDE le carga el firmware al buscar sondas. Si no aparece ninguna, el
  problema es de cable, puerto o placa.
- En el diálogo de sondas, pulsar `Search again`. Si sigue vacío, cerrar las sesiones de
  depuración abiertas y usar el botón rojo *Clean up debug* (termina `redlinkserv`).
- Las reglas udev las instala la IDE (`/lib/udev/rules.d/85-mcuxpresso.rules`). Si faltan,
  reinstalarlas requiere permisos de administrador; después, reconectar la placa.
- Si conecta pero no puede detener el micro, dar reset con P2.10 a GND (modo ISP) y
  reintentar: el micro queda en el cargador y la sonda puede borrar la flash.

## Terminal serie

Conversor USB-UART a 3,3 V: su RX a P0.2 (TXD0), su TX a P0.3 (RXD0) y GND común.
9600 baudios, 8N1, sin control de flujo. Fin de línea al enviar: LF (se admite CR LF).

```sh
picocom -b 9600 --omap crlf /dev/ttyUSB0     # ejemplo; sirve cualquier terminal serie
```

En Linux el usuario debe pertenecer al grupo `dialout` para abrir `/dev/ttyUSB0`.

## Lista de puesta en marcha (I4)

Hacerla en orden, con el motor desconectado hasta el paso 2.

1. **Reloj.** Punto de interrupción después de `Sistema_Init()` en
   `Aplicacion_Inicializar`: `SystemCoreClock == 100000000` y `sistema_listo == true`.
   Si es `false`, el ADC/DMA no entregó el primer par y quedan LED y buzzer activos.
2. **PWM.** Llave abierta (MANUAL). Osciloscopio en P2.0: 1 kHz, y el ciclo de trabajo
   sigue al potenciómetro (P0.23) de 0 a 100 %.
3. **Telemetría.** Cada 500 ms llega `RPM=<n>.<3d>;TEMP=<n>.<2d>;ESTADO=MANUAL`. La
   temperatura debe ser la ambiente (LM35 en P0.24).
4. **Sensor.** Con el motor girando, el contador de `TIMER1` avanza con los pulsos de
   P1.18 y las RPM informadas coinciden con el giro (60 pulsos por vuelta; el motor no
   pasa de unas 185 RPM).
5. **Automático.** Cerrar la llave (P2.6 a GND): `ESTADO=AUTO_ESPERA` y PWM en 0. Enviar
   `RPM 100`: pasa a `AUTO_ACTIVO` y la velocidad se acerca a 100. Frenar levemente el
   eje y ver que el mando lo compensa. Se aceptan consignas de 0 a 185.
6. **Alarma.** Calentar el LM35: por encima de 45,00 °C se encienden el LED (P0.22) y
   el buzzer (P2.7); se apagan al bajar a 43,00 °C o menos. La temperatura es el
   promedio de 4 lecturas, así que responde en unos 2 s.

Anotar en cada paso lo medido: es la evidencia física que todavía no existe.

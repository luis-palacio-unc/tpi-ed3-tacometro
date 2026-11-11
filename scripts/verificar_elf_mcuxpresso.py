#!/usr/bin/env python3
"""Verificacion estatica del .axf generado por MCUXpresso (I3); no ejecuta ni programa hardware.

El proyecto MCUXpresso usa cr_startup_lpc175x_6x.c y el script de enlace administrado,
por lo que sus simbolos de arranque difieren de los de la imagen CMake que revisa
verificar_elf.py. La cadena de reloj se comprueba con la misma funcion de aquel script.
"""
import argparse
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile

from verificar_elf import FLASH_END, RAM_END, RAM_START, require, require_clock, tool

VECTORES_PROPIOS = {17: "TIMER0_IRQHandler", 21: "UART0_IRQHandler", 42: "DMA_IRQHandler"}
CRP_OFFSET, CRP_SIN_PROTECCION = 0x2FC, 0xFFFFFFFF


def inspect(elf, prefix):
    symbols, kinds = {}, {}
    for line in tool(prefix, "nm", "--defined-only", elf).splitlines():
        match = re.match(r"^([0-9a-fA-F]+)\s+(\w)\s+(\S+)$", line)
        if match:
            symbols[match[3]], kinds[match[3]] = int(match[1], 16), match[2]
    require(tool(prefix, "nm", "--undefined-only", elf).strip() == "",
            "El ELF tiene simbolos indefinidos.")
    for name in ("main", "ResetISR", "IntDefaultHandler", "_vStackTop", "CRP_WORD",
                 *VECTORES_PROPIOS.values()):
        require(name in symbols, "Falta simbolo: " + name)
    sections = {}
    for line in tool(prefix, "objdump", "-h", elf).splitlines():
        match = re.match(r"\s*\d+\s+(\S+)\s+([\da-fA-F]+)\s+([\da-fA-F]+)\s+([\da-fA-F]+)\s", line)
        if match:
            sections[match[1]] = {"size": int(match[2], 16), "vma": int(match[3], 16),
                                  "lma": int(match[4], 16)}
    text = sections[".text"]
    require(text["vma"] == 0, ".text (con vectores) no empieza en 0.")
    with tempfile.TemporaryDirectory(prefix="ed3-axf-") as directory:
        binary = Path(directory) / "imagen.bin"
        tool(prefix, "objcopy", "-O", "binary", elf, binary)
        image = binary.read_bytes()
    require(len(image) <= FLASH_END, "La imagen no entra en flash.")
    words = struct.unpack_from("<51I", image, 0)
    require(words[0] == symbols["_vStackTop"] == RAM_END and RAM_START < words[0],
            "Pila inicial distinta del tope de la SRAM local.")
    require(words[1] == (symbols["ResetISR"] | 1), "El vector de reset no es ResetISR.")
    require(sum(words[:8]) & 0xFFFFFFFF == 0, "Checksum de boot NXP invalido.")
    for index, name in VECTORES_PROPIOS.items():
        require(kinds[name] == "T", "ISR sin definicion fuerte: " + name)
        require(words[index] == (symbols[name] | 1) and
                symbols[name] != symbols["IntDefaultHandler"],
                "Vector %d no apunta a %s." % (index, name))
    for index in range(16, 51):
        if index not in VECTORES_PROPIOS:
            require(words[index] == (symbols["IntDefaultHandler"] | 1),
                    "Vector %d con manejador no previsto." % index)
    crp = struct.unpack_from("<I", image, CRP_OFFSET)[0]
    require(symbols["CRP_WORD"] == CRP_OFFSET and crp == CRP_SIN_PROTECCION,
            "CRP ausente o con proteccion activa: 0x%08X" % crp)
    reset = tool(prefix, "objdump", "-d", "--disassemble=ResetISR", elf)
    calls = re.findall(r"\sbl?(?:\.w)?\s+[0-9a-f]+\s+<(\w+)>", reset)
    require("SystemInit" not in calls,
            "ResetISR llama SystemInit: __USE_CMSIS/__USE_LPCOPEN no deben definirse.")
    require("main" in calls, "ResetISR no llama a main.")
    require_clock(elf, prefix, symbols, text)
    flotantes = sorted(name for name in symbols if re.match(r"__aeabi_[fd]", name))
    require(not flotantes, "Auxiliares flotantes enlazados: " + ", ".join(flotantes))
    for name in ("printf", "malloc", "free", "_sbrk"):
        require(name not in symbols, "E/S o heap de libc enlazados: " + name)
    flash = max(s["lma"] + s["size"] for n, s in sections.items()
                if n in (".text", ".data", ".ARM.exidx", ".ARM.extab"))
    ram = sum(sections[n]["size"] for n in (".data", ".bss") if n in sections)
    return ("Imagen MCUXpresso LPC1769: verificacion estatica correcta\n"
            "Vectores 17/21/42 -> TIMER0/UART0/DMA_IRQHandler propios; resto por defecto\n"
            "ResetISR no llama SystemInit; main lleva a Reloj_Init como primera llamada\n"
            "CRP en 0x2FC = 0xFFFFFFFF (sin proteccion); checksum de vectores valido\n"
            f"FLASH: {flash}/{FLASH_END} bytes; SRAM local (.data + .bss, sin pila): {ram}/32768 bytes\n"
            + tool(prefix, "size", elf).strip() + "\n"
            "No se ejecuto firmware en hardware ni simulacion.\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path)
    parser.add_argument("--tool-prefix", default="arm-none-eabi-")
    args = parser.parse_args()
    try:
        print(inspect(args.elf, args.tool_prefix), end="")
    except (OSError, ValueError, KeyError, struct.error, subprocess.CalledProcessError) as error:
        print("ERROR: " + str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Verificacion estatica de una imagen LPC1769; no ejecuta ni programa hardware."""
import argparse
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile

FLASH_END = 0x80000
RAM_START, RAM_END = 0x10000000, 0x10008000


def require(condition, message):
    if not condition:
        raise ValueError(message)


def tool(prefix, name, *args):
    env = dict(os.environ, LC_ALL="C")
    return subprocess.run([prefix + name, *map(str, args)], check=True,
                          text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, env=env).stdout


def require_clock(elf, prefix, symbols, text):
    """Exigir Reloj_Init fuerte que escriba FLASHCFG y llame SystemInit y SystemCoreClockUpdate."""
    strong = set(re.findall(r"^[0-9a-fA-F]+\s+T\s+(\S+)$",
                            tool(prefix, "nm", "--defined-only", elf), re.M))
    for name in ("Reloj_Init", "SystemInit", "SystemCoreClockUpdate"):
        require(name in strong, "Falta simbolo fuerte de reloj: " + name)
        require(text["vma"] <= (symbols[name] & ~1) < text["vma"] + text["size"],
                "Simbolo de reloj fuera de texto: " + name)
    require("SystemCoreClock" in symbols, "Falta simbolo de reloj: SystemCoreClock")
    listing = tool(prefix, "objdump", "-d", "--disassemble=Reloj_Init", elf)
    calls = re.findall(r"\sbl?(?:\.w)?\s+[0-9a-f]+\s+<(\w+)>", listing)
    require(calls == ["SystemInit", "SystemCoreClockUpdate"],
            "Reloj_Init no llama SystemInit y SystemCoreClockUpdate en orden: " + ", ".join(calls))
    before = listing.split("<SystemInit>")[0]
    require(re.search(r"\sstr(?:\.w)?\s", before),
            "Reloj_Init no escribe FLASHCFG antes de SystemInit.")
    main_listing = tool(prefix, "objdump", "-d", "--disassemble=main", elf)
    main_calls = re.findall(r"\sbl\s+[0-9a-f]+\s+<(\w+)>", main_listing)
    require(main_calls[:1] == ["Reloj_Init"],
            "main no llama Reloj_Init como primera funcion: " + ", ".join(main_calls[:3]))


def inspect(elf, prefix, exigir_reloj=False):
    header = elf.read_bytes()[:52]
    require(len(header) == 52 and header[:7] == b"\x7fELF\x01\x01\x01",
            "Se requiere ELF32 little-endian completo.")
    require(struct.unpack_from("<HH", header, 16) == (2, 40),
            "Se requiere ELF ejecutable ARM (no host/objeto).")
    entry = struct.unpack_from("<I", header, 24)[0]
    attributes = tool(prefix, "readelf", "-A", elf)
    require(re.search(r"Tag_CPU_arch:\s+v7\s*(?:\n|$)", attributes)
            and "Tag_CPU_arch_profile: Microcontroller" in attributes
            and "Tag_THUMB_ISA_use: Thumb-2" in attributes,
            "Atributos incompatibles con Cortex-M3/Thumb-2.")
    symbols = {}
    for line in tool(prefix, "nm", "--defined-only", elf).splitlines():
        match = re.match(r"^([0-9a-fA-F]+)\s+\w\s+(\S+)$", line)
        if match:
            symbols[match[2]] = int(match[1], 16)
    names = ("main", "Reset_Handler", "Default_Handler", "__stack_top__",
             "__stack_bottom__", "__data_load__", "__data_start__", "__data_end__",
             "__bss_start__", "__bss_end__")
    require(all(name in symbols for name in names), "Faltan simbolos de arranque.")
    sections = {}
    lines = tool(prefix, "objdump", "-h", elf).splitlines()
    for index, line in enumerate(lines):
        match = re.match(r"\s*\d+\s+(\S+)\s+([\da-fA-F]+)\s+([\da-fA-F]+)\s+([\da-fA-F]+)\s+([\da-fA-F]+)\s+", line)
        if match:
            sections[match[1]] = {"size": int(match[2], 16),
                                  "vma": int(match[3], 16),
                                  "lma": int(match[4], 16),
                                  "flags": lines[index + 1]}
    for name in (".isr_vector", ".text", ".crp", ".stack"):
        require(name in sections, "Falta seccion " + name)
    allowed = {".isr_vector", ".crp", ".text", ".ARM.extab", ".ARM.exidx",
               ".data", ".bss", ".stack"}
    ranges, loads = [], []
    flash_end = 0
    ram_used = 0
    for name, section in sections.items():
        size, vma, lma = (section[key] for key in ("size", "vma", "lma"))
        if not size or "ALLOC" not in section["flags"]:
            continue
        require(name in allowed, "Seccion asignada no prevista: " + name)
        in_ram = name in (".data", ".bss", ".stack")
        lo, hi = (RAM_START, RAM_END) if in_ram else (0, FLASH_END)
        require(lo <= vma and vma + size <= hi, "Seccion fuera de memoria: " + name)
        ranges.append((vma, vma + size, name))
        if in_ram:
            ram_used += size
        if "LOAD" in section["flags"]:
            require(0 <= lma and lma + size <= FLASH_END, "Carga fuera de flash: " + name)
            loads.append((lma, lma + size, name))
            flash_end = max(flash_end, lma + size)
    for group in (ranges, loads):
        ordered = sorted(group)
        require(all(a[1] <= b[0] for a, b in zip(ordered, ordered[1:])),
                "Secciones superpuestas.")
    vectors = sections[".isr_vector"]
    require(vectors["vma"] == 0 and vectors["size"] == 204,
            "Ubicacion/tamano de vectores incorrectos.")
    require(sections[".crp"]["vma"] == 0x2FC and sections[".crp"]["size"] == 4,
            "Ubicacion CRP incorrecta.")
    with tempfile.TemporaryDirectory(prefix="ed3-elf-") as directory:
        vector_file, crp_file = Path(directory)/"vectores", Path(directory)/"crp"
        tool(prefix, "objcopy", "--dump-section", ".isr_vector=" + str(vector_file),
             "--dump-section", ".crp=" + str(crp_file), elf, Path(directory)/"copia.elf")
        vector_bytes = vector_file.read_bytes()
        require(len(vector_bytes) == 204, "Contenido de vectores incompleto.")
        words = struct.unpack("<51I", vector_bytes)
        require(crp_file.read_bytes() == b"\xff" * 4, "CRP no esta desactivado.")
    require(sum(words[:8]) & 0xFFFFFFFF == 0, "Checksum de boot NXP invalido.")
    require(RAM_START < words[0] <= RAM_END and words[0] % 8 == 0
            and words[0] == symbols["__stack_top__"], "Pila inicial invalida.")
    text = sections[".text"]
    for index, address in enumerate(words[1:], 1):
        if index in (7, 8, 9, 10, 13):
            if index != 7:
                require(address == 0, "Vector reservado no es cero.")
            continue
        require(address & 1 and text["vma"] <= (address & ~1) < text["vma"] + text["size"],
                "Vector fuera de texto Thumb en flash: " + str(index))
    require(words[1] == (symbols["Reset_Handler"] | 1) and entry == words[1],
            "Punto de entrada no coincide con Reset_Handler.")
    for name in ("main", "Default_Handler"):
        require(text["vma"] <= (symbols[name] & ~1) < text["vma"] + text["size"],
                "Simbolo fuera de texto: " + name)
    ds, de = symbols["__data_start__"], symbols["__data_end__"]
    bs, be = symbols["__bss_start__"], symbols["__bss_end__"]
    bottom, top = symbols["__stack_bottom__"], symbols["__stack_top__"]
    require(RAM_START <= ds <= de <= bs <= be <= bottom < top == RAM_END,
            "Limites .data/.bss/pila incoherentes.")
    require(all(value % 4 == 0 for value in (ds, de, bs, be, symbols["__data_load__"])),
            "Limites de inicializacion no alineados.")
    for name, start, end in ((".data", ds, de), (".bss", bs, be), (".stack", bottom, top)):
        section = sections.get(name)
        require((not section and start == end) or
                (section and section["vma"] == start and section["size"] == end - start),
                "Simbolos no coinciden con seccion " + name)
    require(top - bottom == 2048, "Reserva de pila distinta de 2 kB.")
    if de > ds:
        require(sections[".data"]["lma"] == symbols["__data_load__"], "LMA .data incoherente.")
    if exigir_reloj:
        require_clock(elf, prefix, symbols, text)
    size_output = tool(prefix, "size", elf).strip()
    return ("Imagen LPC1769: verificacion estatica correcta\n"
            f"FLASH (huella hasta fin de imagen): {flash_end}/{FLASH_END} bytes\n"
            f"SRAM local (.data + .bss + pila): {ram_used}/32768 bytes\n"
            f"Pila reservada: {top-bottom} bytes; no acredita profundidad real\n"
            "AHB0/AHB1: sin secciones asignadas en B00\n" + size_output + "\n"
            + ("Reloj: main llama primero Reloj_Init, que fija FLASHCFG y llama SystemInit\n"
               if exigir_reloj else "")
            + "No se ejecuto firmware en hardware ni simulacion.\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path)
    parser.add_argument("--tool-prefix", default="arm-none-eabi-")
    parser.add_argument("--report", type=Path)
    parser.add_argument("--exigir-reloj", action="store_true",
                        help="exigir Reloj_Init como primera llamada de main, con SystemInit")
    args = parser.parse_args()
    try:
        report = inspect(args.elf, args.tool_prefix, args.exigir_reloj)
        if args.report:
            args.report.write_text(report)
        print(report, end="")
    except (OSError, ValueError, struct.error, subprocess.CalledProcessError) as error:
        print("ERROR: " + str(error), file=sys.stderr)
        if isinstance(error, subprocess.CalledProcessError):
            print(error.stderr, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())

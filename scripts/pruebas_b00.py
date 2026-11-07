#!/usr/bin/env python3
"""Integracion host de build/ELF; no ejecuta codigo ARM ni accede a dispositivos."""
import argparse
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--source", type=Path, required=True)
parser.add_argument("--elf", type=Path, required=True)
parser.add_argument("--tool-prefix", default="arm-none-eabi-")
args, unittest_args = parser.parse_known_args()


class ContratosB00(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="ed3-b00-test-")
        self.addCleanup(self.directory.cleanup)
        self.tmp = Path(self.directory.name)

    def run_command(self, command, expected=0, diagnostic=None):
        result = subprocess.run(list(map(str, command)), text=True,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                env=dict(os.environ, LC_ALL="C"))
        if expected == 0:
            self.assertEqual(result.returncode, 0, result.stdout)
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout)
        if diagnostic:
            self.assertRegex(result.stdout, diagnostic)
        return result.stdout

    def verify(self, elf, expected=0, diagnostic=None):
        return self.run_command([sys.executable, args.source/"scripts/verificar_elf.py",
                                 elf, "--tool-prefix", args.tool_prefix], expected, diagnostic)

    def mutate_section(self, name, data):
        content = self.tmp/"section.bin"
        content.write_bytes(data)
        target = self.tmp/"alterado.elf"
        self.run_command([args.tool_prefix+"objcopy", "--update-section",
                          name+"="+str(content), args.elf, target])
        return target

    def vector_words(self):
        binary = self.tmp/"vectors.bin"
        self.run_command([args.tool_prefix+"objcopy", "--dump-section",
                          ".isr_vector="+str(binary), args.elf, self.tmp/"lectura.elf"])
        return list(struct.unpack("<51I", binary.read_bytes()))

    def altered_vector(self, index, value, repair_checksum=True):
        words = self.vector_words()
        words[index] = value
        if repair_checksum:
            words[7] = (-sum(words[:7])) & 0xFFFFFFFF
        return self.mutate_section(".isr_vector", struct.pack("<51I", *words))

    def fixture(self, source, expected=0, diagnostic=None):
        fixture = self.tmp/"fixture.c"
        fixture.write_text(source)
        startup = self.tmp/"startup.o"
        code = self.tmp/"fixture.o"
        flags = ["-mcpu=cortex-m3", "-mthumb", "-mfloat-abi=soft", "-ffreestanding",
                 "-ffunction-sections", "-fdata-sections", "-Wa,--noexecstack"]
        self.run_command([args.tool_prefix+"gcc", *flags, "-c",
                          args.source/"plataforma/startup_lpc1769.S", "-o", startup])
        self.run_command([args.tool_prefix+"gcc", *flags, "-std=c11", "-Wall", "-Wextra",
                          "-Werror", "-c", fixture, "-o", code])
        elf = self.tmp/"fixture.elf"
        self.run_command([args.tool_prefix+"gcc", *flags, "-nostdlib",
                          "-T", args.source/"plataforma/lpc1769.ld", "-Wl,--gc-sections",
                          "-Wl,--build-id=none", startup, code, "-o", elf], expected, diagnostic)
        return elf

    def test_imagen_valida(self):
        before = (args.elf.read_bytes(), args.elf.stat().st_mtime_ns)
        self.verify(args.elf, diagnostic="verificacion estatica correcta")
        self.assertEqual(args.elf.read_bytes(), before[0], "Contenido del ELF modificado")
        self.assertEqual(args.elf.stat().st_mtime_ns, before[1], "Fecha del ELF modificada")

    def test_elf_truncado(self):
        target = self.tmp/"truncado.elf"
        target.write_bytes(args.elf.read_bytes()[:20])
        self.verify(target, 1, "ELF32")

    def test_arquitectura_no_arm(self):
        data = bytearray(args.elf.read_bytes())
        struct.pack_into("<H", data, 18, 62)
        target = self.tmp/"host.elf"
        target.write_bytes(data)
        self.verify(target, 1, "ejecutable ARM")

    def test_checksum_invalido(self):
        words = self.vector_words()
        self.verify(self.altered_vector(7, words[7] ^ 1, False), 1, "Checksum")

    def test_vector_fuera_de_flash_con_checksum_correcto(self):
        self.verify(self.altered_vector(1, 0x90000001), 1, "Vector fuera")

    def test_pila_desalineada_con_checksum_correcto(self):
        self.verify(self.altered_vector(0, 0x10007FFC), 1, "Pila inicial")

    def test_crp_activado(self):
        self.verify(self.mutate_section(".crp", struct.pack("<I", 0x12345678)), 1, "CRP")

    def test_entry_incorrecto(self):
        data = bytearray(args.elf.read_bytes())
        struct.pack_into("<I", data, 24, 5)
        target = self.tmp/"entry.elf"
        target.write_bytes(data)
        self.verify(target, 1, "Punto de entrada")

    def test_toolchain_ausente(self):
        self.run_command(["cmake", "-S", args.source, "-B", self.tmp/"build",
                          "-DTPI_BUILD_ARM=ON"], 1, "TPI_BUILD_ARM requiere")

    def test_directorio_toolchain_invalido(self):
        self.run_command(["cmake", "-S", args.source, "-B", self.tmp/"build",
                          "-DTPI_BUILD_ARM=ON", "-DCMAKE_TOOLCHAIN_FILE="+
                          str(args.source/"cmake/arm-none-eabi.cmake"),
                          "-DARM_TOOLCHAIN_DIR="+str(self.tmp/"inexistente")],
                         1, "Toolchain ARM incompleto")

    def test_desborde_sram(self):
        self.fixture("volatile unsigned char reserva[32769];\n"
                     "int main(void) { return reserva[0]; }\n", 1,
                     "SRAM|overflow|overlap|will not fit")

    def test_main_ausente(self):
        self.fixture("int auxiliar(void) { return 0; }\n", 1, "undefined reference.*main")

    def test_fixture_data_bss_y_llamada_main(self):
        elf = self.fixture("#include <stdint.h>\n"
                           "volatile uint32_t inicial = 0x12345678;\n"
                           "volatile uint32_t cero;\n"
                           "int main(void) { cero = inicial; return 0; }\n")
        self.verify(elf)
        sections = self.run_command([args.tool_prefix+"objdump", "-h", elf])
        self.assertRegex(sections, r"\.data\s+00000004")
        self.assertRegex(sections, r"\.bss\s+00000004")
        disassembly = self.run_command([args.tool_prefix+"objdump", "-d", elf])
        self.assertIn("<Reset_Handler>", disassembly)
        self.assertRegex(disassembly, r"bl\s+[0-9a-f]+\s+<main>")
        self.assertNotIn("<SystemInit>", disassembly)

    def test_objetos_host(self):
        self.assertIsNotNone(shutil.which("cc"), "Se requiere compilador host para S7")
        self.run_command(["cmake", "-S", args.source, "-B", self.tmp/"host",
                          "-DTPI_BUILD_ARM=OFF", "-DCMAKE_C_COMPILER="+str(shutil.which("cc"))])
        self.run_command(["cmake", "--build", self.tmp/"host", "--parallel", "2"])
        objects = list((self.tmp/"host").rglob("*.o"))
        self.assertEqual(len(objects), 18)
        self.assertFalse(any("startup" in p.name for p in objects))


if __name__ == "__main__":
    unittest.main(argv=[sys.argv[0], *unittest_args], verbosity=2)

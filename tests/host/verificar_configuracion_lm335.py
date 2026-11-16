"""Compilacion positiva/negativa de la referencia ADC y del ajuste del LM335."""
import argparse
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compilador", required=True)
    parser.add_argument("--include", required=True)
    args = parser.parse_args()
    macro, diagnostico = "ADC_VREF_MV", "ADC_VREF_MV debe ser positivo"
    positivos = (None, "1", "3300U", "214748364")
    negativos = ("0", "-1", "214748365", "0x7FFFFFFF")
    probar(args, macro, diagnostico, positivos, negativos)
    macro, diagnostico = "LM335_AJUSTE_CENTESIMAS", "LM335_AJUSTE_CENTESIMAS debe estar"
    positivos = (None, "0", "-5000", "5000", "150")
    negativos = ("-5001", "5001", "27315")
    probar(args, macro, diagnostico, positivos, negativos)
    print("PASS: referencia y ajuste; diagnostico especifico verificado.")


def probar(args, macro, diagnostico, positivos, negativos):
    with tempfile.TemporaryDirectory(prefix="ed3-lm335-config-") as scratch:
        root = Path(scratch)
        fuente = root / "probe.c"
        fuente.write_text('#include "Analogica.h"\nint main(void) { return 0; }\n')
        for indice, valor in enumerate(positivos + negativos):
            objeto = root / f"probe-{indice}.o"
            cmd = [args.compilador, "-std=c11", "-pedantic-errors", "-Wall", "-Wextra",
                   "-Werror", "-Wconversion", "-Wshadow", "-Wstrict-prototypes",
                   "-I", args.include, "-c", str(fuente), "-o", str(objeto)]
            if valor is not None:
                cmd.append(f"-D{macro}={valor}")
            resultado = subprocess.run(cmd, text=True, capture_output=True, timeout=15)
            if indice < len(positivos):
                assert resultado.returncode == 0, (cmd, resultado.stderr)
                assert objeto.exists()
            else:
                assert resultado.returncode != 0, (cmd, "se acepto referencia invalida")
                assert diagnostico in resultado.stderr, resultado.stderr
                assert not objeto.exists()
            print(f"PASS {macro}={valor if valor is not None else 'default'} "
                  f"{'aceptado' if indice < len(positivos) else 'rechazado por guard'}")


if __name__ == "__main__":
    main()

"""Oraculo racional independiente para los getters LM35 compilados por referencia."""
import argparse
from fractions import Fraction
from math import floor
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--caso", nargs=2, action="append", required=True,
                        metavar=("PROGRAMA", "VREF_MV"))
    args = parser.parse_args()
    total = 0
    for programa, referencia in args.caso:
        vref = int(referencia)
        proceso = subprocess.run([programa, "--tabla"], text=True, capture_output=True,
                                 timeout=15, check=True)
        assert not proceso.stderr, proceso.stderr
        lineas = proceso.stdout.splitlines()
        assert len(lineas) == 4097, (programa, len(lineas))
        assert lineas[0] == f"VREF {vref}", (programa, lineas[0], vref)
        for codigo, linea in enumerate(lineas[1:]):
            muestra, centesimas = map(int, linea.split())
            assert muestra == codigo, (muestra, codigo)
            ideal = Fraction(codigo * vref, 4096) * Fraction(100, 10)
            esperado = floor(ideal + Fraction(1, 2))
            assert centesimas == esperado, (vref, codigo, centesimas, esperado)
            assert abs(centesimas - ideal) <= Fraction(1, 2)
            total += 1
        print(f"PASS VREF={vref}: 4096 codigos, igualdad exacta y error <=0,005 grados.")
    print(f"PASS: {total} comparaciones exactas contra Fraction/floor; sin tolerancia float.")


if __name__ == "__main__":
    main()

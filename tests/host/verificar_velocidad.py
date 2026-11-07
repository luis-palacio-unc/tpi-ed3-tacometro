"""Oraculo host independiente: entero arbitrario y un consumidor C del servicio."""
import argparse
import itertools
import random
import subprocess

MAX = (1 << 32) - 1


def vectores():
    valores = (0, 1, 2, 3, 12, 60, 185, 200, MAX - 1, MAX)
    tiempos = (0, 1, 2, 3, 20_000, 1_000_000, 999_999_999, 1_000_000_000, MAX)
    factores = (0, 1, 2, 3, 7, 12, 60, 100, 1024, MAX - 1, MAX)
    yield from itertools.product(valores, tiempos, factores)
    azar = random.Random(0xB031)
    for i in range(10000):
        p, t, f = (azar.getrandbits(32) for _ in range(3))
        if i % 3 == 0:
            f = factores[i % len(factores)]
        if i % 5 == 0:
            t = tiempos[i % len(tiempos)]
        yield p, t, f
    for p, f, k in itertools.product((1, 60, MAX), (1, 3, 7, 12, 60, 100, 1024, MAX),
                                      (0, 1, 2, 999, MAX - 1, MAX)):
        frontera = (2 * p * 60_000_000_000) // (f * (2 * k + 1))
        for t in (frontera - 1, frontera, frontera + 1):
            if 1 <= t <= MAX:
                yield p, t, f
    for f in factores[1:]:
        yield f, 1_000_000, f


def esperado(pulsos, us, factor):
    if us == 0:
        return "ERROR_DURACION", 0
    if factor == 0:
        return "ERROR_FACTOR", 0
    n = pulsos * 60_000_000_000
    d = us * factor
    resultado = (2 * n + d) // (2 * d)
    return ("ERROR_RANGO", 0) if resultado > MAX else ("VALIDA", resultado)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--programa", required=True)
    args = parser.parse_args()
    casos = list(vectores())
    entrada = "".join(f"{p} {t} {f}\n" for p, t, f in casos)
    proceso = subprocess.run([args.programa], input=entrada, text=True,
                             capture_output=True, timeout=30, check=True)
    if proceso.stderr:
        raise AssertionError(proceso.stderr)
    lineas = proceso.stdout.splitlines()
    assert len(lineas) == len(casos), (len(lineas), len(casos))
    for caso, linea in zip(casos, lineas):
        estado, valor = linea.split()
        obtenido = estado, int(valor)
        referencia = esperado(*caso)
        assert obtenido == referencia, (caso, obtenido, referencia)
    print(f"PASS: {len(casos)} ternas contra oraculo entero exacto independiente; "
          "fronteras, errores, configuraciones y dominio uint32_t.")


if __name__ == "__main__":
    main()

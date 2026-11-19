"""Oraculo racional independiente y planta de prueba del regulador PI.

Cada --caso es un emisor compilado con otras ganancias de Control.h. La planta es
un modelo de prueba, no el motor real.
"""
import argparse
from fractions import Fraction
import json
from math import floor
from pathlib import Path
import subprocess

DEFAULT = (1, 2000, 1, 2500)
HALF = Fraction(1, 2)


class Referencia:
    def __init__(self, config):
        self.kp = Fraction(config[0], config[1])
        self.ki = Fraction(config[2], config[3])
        self.integral = Fraction(0)

    def step(self, goal, value):
        if goal == 0:
            self.integral = Fraction(0)
            return 0
        error = goal * 1000 - value
        v = self.kp * error + self.integral
        output = max(0, min(1000, floor(v + HALF)))
        if not ((v >= 1000 and error > 0) or (v <= 0 and error < 0)):
            self.integral = max(Fraction(0), min(Fraction(1000), self.integral + self.ki * error))
        return output


def rows_for(config):
    rows = []
    add = lambda goal, value=0: rows.append((goal, value))
    add(0)
    if config == DEFAULT:
        # Paso, empate 0,5 y borde del empate.
        add(100); add(100); add(0); add(1, 0); add(0); add(1, 1); add(0)
        # Anti-windup: tres pasos previos, cuarenta saturados y desaturacion.
        for _ in range(43): add(700)
        add(100); add(100, 0xffffffff); add(100); add(0)
        # Mando exactamente cero con error negativo: la integral se conserva.
        for _ in range(4): add(5)
        add(5, 21000); add(5, 5000); add(0)
    if config == (1, 1000, 1, 1000):
        add(1, 500); add(0)
    if config == (1, 1000, 1000, 1):
        # Ki > Kp alcanza los dos limites de la integral.
        add(100); add(100, 150000); add(100, 100000); add(100, 99999); add(0)
        add(100); add(100, 99000); add(100, 100000); add(100, 101000); add(0)
    # Fronteras de consigna y medicion, en secuencia (la integral se arrastra).
    for goal in (0, 1, 699, 700, 701, 0xffffffff):
        for value in (0, 1, 699999, 700000, 700001, 0xffffffff):
            add(goal, value)
    deterministic = len(rows)
    seed = 0x0B170017
    for _ in range(32):
        add(0)
        for _ in range(256):
            words = []
            for _ in range(3):
                seed = (1664525 * seed + 1013904223) & 0xffffffff
                words.append(seed)
            selector = (words[2] >> 8) % 8
            goal = 0 if selector == 0 else 1 + words[0] % 701
            if selector < 2:
                value = words[1]
            elif selector < 5:
                value = words[1] % 700001
            else:
                # Cerca de la consigna: regimen sin saturar, donde la integral trabaja.
                value = max(0, goal * 1000 + words[1] % 4001 - 2000)
            add(goal, value)
    return rows, deterministic


def run(program, rows):
    inputs = "".join(f"{g} {v}\n" for g, v in rows)
    result = subprocess.run([program], input=inputs, text=True, capture_output=True, check=False)
    assert result.returncode == 0, f"emisor exit {result.returncode}: {result.stderr}"
    return inputs, [int(line) for line in result.stdout.splitlines()]


def oracle(program, config, evidence):
    rows, deterministic = rows_for(config)
    ref = Referencia(config)
    expected = [ref.step(*row) for row in rows]
    inputs, actual = run(program, rows)
    tag = "-".join(map(str, config))
    (evidence / f"oracle-{tag}-input.txt").write_text(inputs)
    (evidence / f"oracle-{tag}-actual.txt").write_text("".join(f"{x}\n" for x in actual))
    (evidence / f"oracle-{tag}-expected.txt").write_text("".join(f"{x}\n" for x in expected))
    assert len(actual) == len(expected), (config, len(actual), len(expected))
    for k, (want, got) in enumerate(zip(expected, actual)):
        assert got == want, f"oraculo {config} paso {k} entrada={rows[k]} esperado={want} obtenido={got}"
    return dict(config=config, exact_steps=len(rows), deterministic_steps=deterministic)


def plant(program, evidence):
    """Planta lineal con los datos del ensayo del 2026-11-19: 0,73 RPM por cada por
    mil de mando y una ventana de 500 ms que promedia el 72 % del mando vigente y
    el 28 % del anterior (constante de tiempo de 0,14 s). Sin zona muerta."""
    ref = Referencia(DEFAULT)
    process = subprocess.Popen([program], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, text=True, bufsize=1)
    trace = []
    x = 0
    u1 = u2 = 0
    try:
        for k in range(200):
            goal = 300 if k < 100 else 600
            measured = 2000 * (x // 2000)
            process.stdin.write(f"{goal} {measured}\n")
            process.stdin.flush()
            u = int(process.stdout.readline())
            assert u == ref.step(goal, measured), ("planta", k, goal, measured, u)
            u2, u1 = u1, u
            x = 730 * (72 * u1 + 28 * u2) // 100
            trace.append(dict(k=k, goal=goal, measured=measured, u=u, x_next=x))
    finally:
        process.stdin.close()
        stdout_tail = process.stdout.read()
        errors = process.stderr.read()
        code = process.wait()
    assert code == 0 and not stdout_tail, (code, errors, stdout_tail)
    (evidence / "plant-trace.json").write_text(json.dumps(trace, indent=2) + "\n")
    metrics = []
    for start, goal, amplitude in ((0, 300, 300), (100, 600, 300)):
        values = [p["x_next"] for p in trace[start:start + 100]]
        peak = max(values)
        excess = max(0, peak - goal * 1000)
        settling = next((j + 1 for j in range(100)
                         if all(abs(v - goal * 1000) <= 6000 for v in values[j:])), None)
        print("planta", goal, "pico", peak, "establecimiento", settling)
        assert 10 * excess <= amplitude * 1000, ("sobrepico", goal, peak)
        assert settling is not None and settling <= 20, ("establecimiento", goal, settling)
        metrics.append(dict(goal_rpm=goal, peak_milli_rpm=peak, overshoot_milli_rpm=excess,
                            settling_steps=settling, horizon_steps=100))
    print("PASS planta de prueba (modelo lineal del motor):", metrics)
    return metrics


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--caso", nargs=5, action="append", required=True,
                        metavar=("PROGRAMA", "KP_NUM", "KP_DEN", "KI_NUM", "KI_DEN"))
    parser.add_argument("--evidencia", type=Path, required=True)
    args = parser.parse_args()
    args.evidencia.mkdir(parents=True, exist_ok=True)
    summary = dict(oracle=[])
    default_program = None
    for program, *gains in args.caso:
        config = tuple(map(int, gains))
        program = str(Path(program).resolve())
        summary["oracle"].append(oracle(program, config, args.evidencia))
        if config == DEFAULT:
            default_program = program
    print("PASS oraculo:", len(summary["oracle"]), "configuraciones,",
          sum(c["exact_steps"] for c in summary["oracle"]), "pasos exactos")
    assert default_program is not None, "falta el caso con las ganancias por defecto"
    summary["plant_metrics"] = plant(default_program, args.evidencia)
    (args.evidencia / "results.json").write_text(json.dumps(summary, indent=2) + "\n")


if __name__ == "__main__":
    main()

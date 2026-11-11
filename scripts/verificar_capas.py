#!/usr/bin/env python3
"""Control de capas: Aplicacion -> Servicios -> Dispositivos, por los #include propios."""
import argparse
from pathlib import Path
import re
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--source", type=Path, required=True)
args = parser.parse_args()

CAPAS = {
    "Aplicacion": {"Aplicacion", "Servicios", "Configuracion"},
    "Servicios": {"Servicios", "Dispositivos", "Dispositivos/soporte", "Configuracion"},
}
PROPIAS = ("Aplicacion", "Servicios", "Dispositivos", "Dispositivos/soporte", "Configuracion")
propietario = {}
for capa in PROPIAS:
    for encabezado in (args.source / capa).glob("*.h"):
        propietario[encabezado.name] = capa

errores = []
revisados = 0
for capa, permitidas in CAPAS.items():
    archivos = sorted((args.source / capa).glob("*.[ch]"))
    if not archivos:
        errores.append(f"{capa}: sin archivos para revisar")
    for archivo in archivos:
        for numero, linea in enumerate(archivo.read_text().splitlines(), 1):
            incluido = re.match(r'\s*#\s*include\s*([<"])([^>"]+)[>"]', linea)
            if not incluido:
                continue
            revisados += 1
            nombre = incluido[2]
            if incluido[1] == "<":
                if not re.fullmatch(r"std(bool|int|def)\.h", nombre):
                    errores.append(f"{archivo.name}:{numero}: encabezado de sistema no previsto <{nombre}>")
            elif "/" in nombre or nombre not in propietario:
                errores.append(f"{archivo.name}:{numero}: \"{nombre}\" no es un encabezado propio conocido")
            elif propietario[nombre] not in permitidas:
                errores.append(f"{archivo.name}:{numero}: {capa} no puede incluir "
                               f"\"{nombre}\" de {propietario[nombre]}")
if errores:
    print("\n".join(errores), file=sys.stderr)
    sys.exit(1)
print(f"Capas correctas: {revisados} inclusiones de Aplicacion y Servicios revisadas.")

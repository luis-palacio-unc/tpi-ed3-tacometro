"""Inspeccion estatica del vector SysTick B11; no ejecuta firmware ARM."""
import argparse
from pathlib import Path
import struct
import subprocess
import tempfile


def verificar(elf, prefix, api):
    salida = subprocess.check_output([prefix+'nm', '-P', '--defined-only', str(elf)], text=True)
    simbolos = {p[0]: (p[1], int(p[2], 16)) for line in salida.splitlines()
                if len(p := line.split()) >= 3}
    tipo, direccion = simbolos.get('SysTick_Handler', ('?', 0))
    if tipo != 'T':
        raise ValueError('SysTick_Handler debe ser fuerte (T), no alias debil por defecto.')
    with tempfile.TemporaryDirectory(prefix='ed3-vector-b11-') as directorio:
        destino = Path(directorio)/'vectores.bin'
        copia = Path(directorio)/'lectura.elf'
        subprocess.run([prefix+'objcopy', '--dump-section', '.isr_vector='+str(destino),
                        str(elf), str(copia)], check=True)
        datos = destino.read_bytes()
    if len(datos) != 204 or struct.unpack_from('<I', datos, 60)[0] != direccion | 1:
        raise ValueError('Vector 15 no apunta al SysTick_Handler fuerte.')
    if direccion == simbolos['Default_Handler'][1]:
        raise ValueError('Handler SysTick coincide con Default_Handler.')
    if api:
        for nombre in ('Base_Tiempos_Inicializar', 'Base_Tiempos_Leer', 'Base_Tiempos_Detener',
                       'Base_Tiempos_Puerto_EntrarCritica', 'Base_Tiempos_Puerto_SalirCritica',
                       'Base_Tiempos_Puerto_Escribir', 'Mandos_InicializarSelector', 'Mandos_ActualizarSelector'):
            if simbolos.get(nombre, ('?', 0))[0] != 'T':
                raise ValueError('API B11/B10 ausente: '+nombre)
    print('PASS vector SysTick fuerte'+(' y API B11/B10 retenida' if api else '')+'; inspeccion ARM, sin ejecucion.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('elf', type=Path)
    parser.add_argument('--tool-prefix', default='arm-none-eabi-')
    parser.add_argument('--api', action='store_true')
    args = parser.parse_args()
    try:
        verificar(args.elf, args.tool_prefix, args.api)
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        parser.exit(1, str(error)+'\n')

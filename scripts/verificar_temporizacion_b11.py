"""Inspeccion estatica B11 TC: sin ISR propia y APIs retenidas; no ejecuta ARM."""
import argparse
from pathlib import Path
import struct
import subprocess
import tempfile


def verificar(elf,prefix,api):
    salida=subprocess.check_output([prefix+'nm','-P','--defined-only',str(elf)],text=True)
    simbolos={p[0]:(p[1],int(p[2],16)) for line in salida.splitlines() if len(p:=line.split())>=3}
    default=simbolos.get('Default_Handler',('?',0))
    if default[0]!='T':
        raise ValueError('Default_Handler ausente.')
    with tempfile.TemporaryDirectory(prefix='ed3-b11-tc-vector-') as directorio:
        destino=Path(directorio)/'vectores.bin'
        subprocess.run([prefix+'objcopy','--dump-section','.isr_vector='+str(destino),
                        str(elf),str(Path(directorio)/'lectura.elf')],check=True)
        datos=destino.read_bytes()
    if len(datos)!=204:
        raise ValueError('Tabla de vectores LPC1769 incorrecta.')
    for nombre,indice in (('SysTick_Handler',15),('TIMER0_IRQHandler',17)):
        tipo,direccion=simbolos.get(nombre,('?',0))
        if tipo!='W' or direccion!=default[1] or struct.unpack_from('<I',datos,indice*4)[0]!=(direccion|1):
            raise ValueError(nombre+' debe conservar alias debil por defecto, sin ISR propia B11.')
    if api:
        for nombre in ('Base_Tiempos_Inicializar','Base_Tiempos_Leer','Base_Tiempos_Detener',
                       'Base_Tiempos_ProveerMilisegundos','Base_Tiempos_Puerto_Preparar',
                       'Base_Tiempos_Puerto_Iniciar','Base_Tiempos_Puerto_LeerTC','Base_Tiempos_Puerto_Detener',
                       'Tiempo_Leer','Tiempo_Transcurrido','Mandos_InicializarSelector','Mandos_ActualizarSelector'):
            if simbolos.get(nombre,('?',0))[0]!='T':
                raise ValueError('API B11/B10 ausente: '+nombre)
    print('PASS B11 TC sin ISR propia'+(' y APIs retenidas' if api else '')+'; inspeccion ARM, no ejecucion.')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('elf',type=Path)
    parser.add_argument('--tool-prefix',default='arm-none-eabi-')
    parser.add_argument('--api',action='store_true')
    args=parser.parse_args()
    try:
        verificar(args.elf,args.tool_prefix,args.api)
    except (ValueError,OSError,subprocess.CalledProcessError) as error:
        parser.exit(1,str(error)+'\n')

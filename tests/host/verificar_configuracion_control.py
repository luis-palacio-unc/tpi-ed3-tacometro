"""Compilar probes de configuracion sin modificar definiciones del firmware."""
import argparse
import json
from pathlib import Path
import re
import subprocess

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--compilador",required=True)
    parser.add_argument("--include",required=True)
    parser.add_argument("--evidencia",type=Path,required=True)
    a=parser.parse_args()
    a.evidencia.mkdir(parents=True,exist_ok=True)
    source=a.evidencia/"probe.c"
    source.write_text('#include "Control.h"\n#include <stdio.h>\nint main(void) {\n printf("%u %u %u %u %u\\n", (unsigned)TPI_CONTROL_KP_NUM,\n(unsigned)TPI_CONTROL_KP_DEN,(unsigned)TPI_CONTROL_KI_NUM,(unsigned)TPI_CONTROL_KI_DEN,\n(unsigned)TPI_CONTROL_PERIODO_MS); return 0; }\n')
    names=["TPI_CONTROL_KP_NUM","TPI_CONTROL_KP_DEN","TPI_CONTROL_KI_NUM","TPI_CONTROL_KI_DEN",
           "TPI_CONTROL_PERIODO_MS"]
    positives=[None,(1,1,1,1,500),(1000,1000,1000,1000,500),(1000,1,1000,1,500)]
    records=[]
    def compile_one(label,defines):
        binary=a.evidencia/label
        cmd=[a.compilador,"-std=c11","-Wall","-Wextra","-Werror","-pedantic-errors",
             "-I"+a.include,*["-D"+k+"="+str(v) for k,v in defines.items()],str(source),"-o",str(binary)]
        p=subprocess.run(cmd,text=True,capture_output=True,check=False)
        (a.evidencia/(label+".log")).write_text(p.stdout+p.stderr)
        records.append(dict(label=label,args=cmd,exit=p.returncode))
        (a.evidencia/"commands.json").write_text(json.dumps(records,indent=2)+"\n")
        return p,binary
    for i,values in enumerate(positives):
        p,binary=compile_one("positive"+str(i),dict(zip(names,values)) if values else {})
        assert p.returncode==0,p.stderr
        result=subprocess.run([str(binary)],text=True,capture_output=True,check=False)
        assert result.returncode==0,result.stderr
        (a.evidencia/("positive"+str(i)+"-run.log")).write_text(result.stdout+result.stderr)
        assert tuple(map(int,result.stdout.split()))==(values or (1,2000,1,2500,500))
    negatives=[]
    for name in names[:4]:
        maximo=10001 if name.endswith("_DEN") else 1001
        negatives.extend(({name:v},name) for v in (0,-1,maximo,"UINT32_MAX"))
    negatives.extend(({names[4]:v},names[4]) for v in (499,1000))
    # Cada componente en rango, pero KP_NUM * KI_DEN desborda la cuenta de 64 bits.
    negatives.append(({names[0]:1000,names[3]:1001},names[0]))
    assert len(negatives)==19
    for i,(defines,name) in enumerate(negatives):
        p,_=compile_one("negative"+str(i),defines)
        assert p.returncode!=0,defines
        assert re.search(r"#error[^\n]*"+name,p.stderr),(defines,p.stderr)
    print("PASS S6: 4 configuraciones positivas y 19 negativas; diagnostico especifico")
    (a.evidencia/"results.json").write_text(json.dumps(dict(positive=4,negative=19),indent=2)+"\n")
if __name__=="__main__":
    main()

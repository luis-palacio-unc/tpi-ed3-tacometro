"""Verificacion reproducible B17; evidencia local ignorada, sin hardware ni red."""
import argparse
import difflib
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import xml.etree.ElementTree as ET

ROOT=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser()
parser.add_argument("--evidencia",type=Path,default=ROOT/"build/verificacion-b17-2026-11-11")
parser.add_argument("--fase",choices=("all","host","san","arm","docs","mutations","resources","preservation"),default="all")
args=parser.parse_args()
OUT=args.evidencia.resolve()
assert ROOT in OUT.parents and OUT!=ROOT,"La evidencia debe quedar dentro del proyecto."
OUT.mkdir(parents=True,exist_ok=True)
PHASE=None
RECORDS=[]

def run(command,label,expected=0,data=None,env=None):
    dest=PHASE/(label+".log")
    with dest.open("w") as stream:
        p=subprocess.run([str(x) for x in command],cwd=ROOT,input=data,text=True,env=env,stdout=stream,stderr=subprocess.STDOUT)
    RECORDS.append(dict(command=[str(x) for x in command],exit=p.returncode,expected=expected,
                        log=str(dest.relative_to(OUT)),
                        env={k:env[k] for k in ("ASAN_OPTIONS","UBSAN_OPTIONS") if env and k in env}))
    (PHASE/"commands.json").write_text(json.dumps(RECORDS,indent=2)+"\n")
    assert p.returncode==expected,f"{label}: exit{p.returncode}, esperado{expected}: "+dest.read_text()[-3000:]
    return dest.read_text()

def build(mode):
    build_dir=OUT/mode
    command=["cmake","-S",ROOT,"-B",build_dir,"-DCMAKE_BUILD_TYPE=Debug","-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"]
    if mode=="san":command+=["-DTPI_HOST_SANITIZERS=ON"]
    if mode=="arm":command+=["-DTPI_BUILD_ARM=ON",f"-DCMAKE_TOOLCHAIN_FILE={ROOT}/cmake/arm-none-eabi.cmake"]
    run(command,"configure")
    run(["cmake","--build",build_dir,"--parallel","4"],"build")
    temporary=OUT/"temporary"
    temporary.mkdir(exist_ok=True)
    env=dict(os.environ,TMPDIR=str(temporary))
    if mode=="san":
        env.update(ASAN_OPTIONS="detect_leaks=1:halt_on_error=1",UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1")
    log=run(["ctest","--test-dir",build_dir,"--verbose"],"tests",env=env)
    count=15 if mode=="arm" else 33
    assert f"100% tests passed, 0 tests failed out of {count}" in log
    assert not re.search(r"Skipped|Not Run",log)
    return dict(passed=count,skipped=0)

def docs():
    config=(ROOT/"docs/doxygen/Doxyfile").read_text()+f"\nOUTPUT_DIRECTORY = {PHASE}/doxygen\n"
    log=run(["doxygen","-"],"doxygen",data=config)
    assert not log.strip(),log
    functions={}
    for p in (PHASE/"doxygen/xml").glob("*.xml"):
        for member in ET.parse(p).iterfind('.//memberdef[@kind="function"]'):
            functions[member.findtext("name")]=member
    assert len(functions)==46,len(functions)
    for name,num in (("Procesamiento_InicializarRegulador",8),("Procesamiento_Regular",9),("Procesamiento_ReiniciarRegulador",10)):
        assert f"USW.2.1.2.{num}" in " ".join(functions[name].itertext())
    assert (PHASE/"doxygen/html/index.html").exists()
    return dict(documented_functions=46,warnings=0)

STRICT=["-std=c11","-Wall","-Wextra","-Werror","-Wconversion","-Wshadow","-Wstrict-prototypes","-pedantic-errors"]
def mutations():
    original=(ROOT/"Aplicacion/Procesamiento.c").read_text()
    replacements={
        "correct":original,
        "anti-windup":original.replace("if (valida && !saturando)","(void)saturando;\n    if (valida)"),
        "rounding":original.replace("(n + d / 2) / d","n / d"),
        "integral-upper":original.replace(
            "r->integral_num = siguiente < 0 ? 0 : (siguiente > max_integral ? max_integral : siguiente);",
            "(void)max_integral;\n        r->integral_num = siguiente < 0 ? 0 : siguiente;"),
        "integral-lower":original.replace(
            "r->integral_num = siguiente < 0 ? 0 : (siguiente > max_integral ? max_integral : siguiente);",
            "r->integral_num = siguiente > max_integral ? max_integral : siguiente;"),
        "freeze-zero":original.replace("(n <= 0 && error < 0)","(n < 0 && error < 0)")
    }
    assert all(s!=original for name,s in replacements.items() if name!="correct")
    summary={}
    for name,source in replacements.items():
        copy=PHASE/name
        for relative in ("Aplicacion/Procesamiento.h","Configuracion/Control.h","Configuracion/Limites.h",
                         "Servicios/Temperatura.h","Servicios/Medicion_Velocidad.h",
                         "tests/host/test_regulador.c","tests/host/consulta_regulador.c","tests/host/verificar_regulador.py"):
            dest=copy/relative;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/relative,dest)
        (copy/"Aplicacion/Procesamiento.c").write_text(source)
        (copy/"mutation.diff").write_text("".join(difflib.unified_diff(original.splitlines(True),source.splitlines(True),fromfile="correct",tofile=name)))
        includes=[f"-I{copy/p}" for p in ("Aplicacion","Servicios","Configuracion")]
        for kind,unit in (("unit","test_regulador.c"),("oracle","consulta_regulador.c")):
            run(["cc",*STRICT,*includes,copy/"tests/host"/unit,copy/"Aplicacion/Procesamiento.c","-o",copy/kind],name+"-compile-"+kind)
        exit_code=0 if name=="correct" else 1
        unit_log=run([copy/"unit"],name+"-unit",expected=exit_code)
        oracle_log=run(["python3",copy/"tests/host/verificar_regulador.py","--programa",copy/"oracle",
                        "--evidencia",copy/"oracle-evidence","--solo-oraculo"],name+"-oracle",expected=exit_code)
        if name!="correct":
            expected_failure={
                "anti-windup":"FAIL S2 anti-windup", "rounding":"FAIL S1 paso/empate",
                "integral-upper":"FAIL S2 limites integral Ki>Kp",
                "integral-lower":"FAIL S2 limites integral Ki>Kp",
                "freeze-zero":"FAIL S2 n0 e negativo"
            }[name]
            assert expected_failure in unit_log
            assert "S9 paso" in oracle_log and "esperado=" in oracle_log and "obtenido=" in oracle_log
        summary[name]=dict(compiled=True,unit_exit=exit_code,oracle_exit=exit_code,
                           detected=name!="correct")
    assert (ROOT/"Aplicacion/Procesamiento.c").read_text()==original
    return summary

def resources():
    inc=[f"-I{ROOT/p}" for p in ("Aplicacion","Servicios","Configuracion")]
    arm=["arm-none-eabi-gcc","-mcpu=cortex-m3","-mthumb","-mfloat-abi=soft",*STRICT,*inc,"-ffreestanding","-fno-unwind-tables","-fno-asynchronous-unwind-tables"]
    size_source=PHASE/"size.c"
    size_source.write_text('#include "Procesamiento.h"\nconst unsigned char b17_tamano_regulador[sizeof(Procesamiento_Regulador)]={0};\n')
    run([*arm,"-c",size_source,"-o",PHASE/"size.o"],"size-compile")
    sizes=run(["arm-none-eabi-nm","-S",PHASE/"size.o"],"size-symbols")
    match=re.search(r"[0-9a-f]+ ([0-9a-f]+) [A-Za-z] b17_tamano_regulador",sizes)
    assert match and int(match[1],16)<=48,sizes
    result=dict(instance_bytes=int(match[1],16),objects={})
    for opt in ("O0","O2"):
        obj=PHASE/(opt+".o")
        run([*arm,"-"+opt,"-fstack-usage","-c",ROOT/"Aplicacion/Procesamiento.c","-o",obj],opt+"-compile")
        symbols=run(["arm-none-eabi-nm","-S","--size-sort",obj],opt+"-symbols")
        assert not re.search(r"__aeabi_[fd]",symbols)
        run(["arm-none-eabi-size","-A",obj],opt+"-sections")
        run(["arm-none-eabi-objdump","-dr",obj],opt+"-disassembly")
        stack=(PHASE/(opt+".su")).read_text()
        result["objects"][opt]=dict(stack_usage=stack,new_function_sizes={})
        for name in ("Procesamiento_InicializarRegulador","Procesamiento_Regular","Procesamiento_ReiniciarRegulador"):
            m=re.search(r"[0-9a-f]+ ([0-9a-f]+) T "+name+r"\b",symbols)
            assert m,(name,symbols)
            result["objects"][opt]["new_function_sizes"][name]=int(m[1],16)
        undefined=run(["arm-none-eabi-nm","--undefined-only",obj],opt+"-undefined")
        assert not re.search(r"__aeabi_[fd]",undefined)
    fixture=OUT/"arm/b17-fixture/fixture.elf"
    assert fixture.exists(),"Ejecutar fase arm antes de resources."
    symbols=run(["arm-none-eabi-nm","-S","--size-sort",fixture],"fixture-symbols")
    run(["arm-none-eabi-objdump","-d",fixture],"fixture-disassembly")
    all_symbols=run(["arm-none-eabi-nm","-S",fixture],"fixture-all-symbols")
    result["integer_helpers"]=[line.strip() for line in all_symbols.splitlines() if "__aeabi_" in line or "__div" in line or "__udiv" in line]
    assert not re.search(r"__aeabi_[fd]",symbols)
    return result

def preservation():
    baseline="9352f0d"
    command=["git","ls-tree","-r","--name-only",baseline,"--",
             "Drivers","Servicios","Dispositivos","Configuracion","Aplicacion/main.c",
             "openspec/specs","openspec/changes/archive"]
    paths=run(command,"baseline-paths").splitlines()
    hashes={}
    for name in paths:
        expected=subprocess.check_output(["git","show",baseline+":"+name],cwd=ROOT)
        assert (ROOT/name).read_bytes()==expected,name
        hashes[name]=hashlib.sha256(expected).hexdigest()
    original_c=subprocess.check_output(["git","show",baseline+":Aplicacion/Procesamiento.c"],cwd=ROOT,text=True)
    current_c=(ROOT/"Aplicacion/Procesamiento.c").read_text().replace('#include "Control.h"\n',"")
    assert current_c.split("bool Procesamiento_ReiniciarRegulador(",1)[0]==original_c.rsplit("/** @} */",1)[0]
    original_h=subprocess.check_output(["git","show",baseline+":Aplicacion/Procesamiento.h"],cwd=ROOT,text=True)
    current_h=(ROOT/"Aplicacion/Procesamiento.h").read_text().replace('#include "Medicion_Velocidad.h"\n',"").replace(
        "servicio generico contra un umbral configurable. B17 agrega PI separado; sin actuacion.",
        "servicio generico contra un umbral configurable. Regulacion/actuacion pendientes.")
    assert current_h.split("/** @brief Ganancias discretas",1)[0]==original_h.rsplit("/** @} */",1)[0]
    run(["git","diff","--check"],"diff-check")
    run(["git","diff",baseline,"--"],"review-diff")
    result=dict(baseline=baseline,preserved_files=len(paths),drivers=sum(x.startswith("Drivers/") for x in paths),
                b01_b06_types_functions_preserved=True,services_devices_specs_archive_main_preserved=True,
                preserved_sha256=hashes)
    (OUT/"preservation.json").write_text(json.dumps(result,indent=2)+"\n")
    (OUT/"review.diff").write_text((PHASE/"review-diff.log").read_text())
    return dict(preserved_files=len(paths),drivers=result["drivers"],b01_b06_preserved=True)

phases=("host","san","arm","docs","mutations","resources","preservation") if args.fase=="all" else (args.fase,)
for phase in phases:
    attempts=sorted(OUT.glob(phase+"-attempt-*"))
    PHASE=OUT/(phase+f"-attempt-{len(attempts)+1:03d}")
    PHASE.mkdir()
    RECORDS=[]
    source_files=subprocess.check_output(["git","ls-files","--cached","--others","--exclude-standard","-z"],cwd=ROOT,text=True).split("\0")
    manifest={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest()
        for name in sorted(set(source_files)) if name and ((ROOT/name).suffix in
        (".c",".h",".S",".ld",".cmake",".py") or name in ("CMakeLists.txt","docs/doxygen/Doxyfile"))}
    (PHASE/"source-inputs.json").write_text(json.dumps(manifest,indent=2)+"\n")
    for tool in ("cc","arm-none-eabi-gcc","cmake","ctest","python3","doxygen"):
        run([tool,"--version"],"version-"+tool)
    result=build(phase) if phase in ("host","san","arm") else globals()[phase]()
    (PHASE/"results.json").write_text(json.dumps(result,indent=2)+"\n")
    (OUT/(phase+"-latest.txt")).write_text(str(PHASE.relative_to(OUT))+"\n")
    print(phase+": PASS "+str(result if phase!="resources" else {"instance_bytes":result["instance_bytes"]}),flush=True)

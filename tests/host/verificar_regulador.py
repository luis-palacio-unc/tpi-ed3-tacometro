"""Oraculo racional independiente y planta de prueba B17 (no modelo del motor real)."""
import argparse
from fractions import Fraction
import json
from math import floor
from pathlib import Path
import subprocess

DEFAULT = (4, 1000, 1, 1000, 4)
CONFIGS = (DEFAULT, (1,1000,1,1000,1), (1000,1,1000,1,4),
           (1,1,1,1,1), (1000,1000,1000,1000,4), (3,7,2,9,2))
HALF = Fraction(1,2)

class Referencia:
    def __init__(self):
        self.config = DEFAULT
        self.enabled = True
        self.reset()

    def reset(self):
        self.integral = Fraction(0)
        self.last = 0
        self.count = 0
        self.has = False

    def step(self, row):
        action, goal, value, state, *cfg = row
        output = 0
        if action == 0:
            self.reset()
            self.enabled = all(1 <= x <= 1000 for x in cfg[:4]) and 1 <= cfg[4] <= 65535
            self.config = tuple(cfg) if self.enabled else (0,0,0,0,0)
            code = int(self.enabled)
        elif action == 1:
            self.reset()
            code = 1
        elif not self.enabled:
            self.reset()
            code = 5
        elif goal > 185:
            self.reset()
            code = 6
        elif goal == 0:
            self.reset()
            code = 3
        elif state != 1 and (not self.has or self.count == self.config[4]):
            self.reset()
            code = 2
        else:
            if state == 1:
                self.last, self.count, self.has = value, 0, True
                code = 0
            else:
                self.count += 1
                code = 1
            error = goal * 1000 - self.last
            p = Fraction(self.config[0], self.config[1]) * error
            v = p + self.integral
            output = max(0, min(1000, floor(v + HALF)))
            if state == 1 and not ((v >= 1000 and error > 0) or (v <= 0 and error < 0)):
                self.integral = max(Fraction(0), min(Fraction(1000),
                    self.integral + Fraction(self.config[2], self.config[3]) * error))
        numerator = self.integral * self.config[3]
        assert numerator.denominator == 1
        return (code,output,int(numerator),self.last,self.count,int(self.has),int(self.enabled))

def actions():
    rows = []
    def add(a, g=0, v=0, s=1, cfg=DEFAULT):
        rows.append((a,g,v,s,*cfg))
    # S1 y m1: empate independiente de los defaults.
    add(0,cfg=(1,1000,1,1000,4))
    add(2,1,500)
    add(0)
    add(2,100); add(2,100)
    # S2: dos pasos previos y cuarenta saturados.
    add(0)
    for _ in range(42): add(2,185)
    add(2,100)
    # Dominio completo de tipos, fronteras de consigna/medicion y estados.
    for cfg in CONFIGS:
        add(0,cfg=cfg)
        for goal in (0,1,184,185,186,0xffffffff):
            for value in (0,1,184999,185000,185001,0xffffffff):
                for state in (0,1,2,3,4,5,6,77):
                    add(2,goal,value,state,cfg)
    # Datos utiles, reinicio, nueva valida dentro de racha y retencion extrema real.
    for limit in (1,4,65535):
        cfg=(*DEFAULT[:4],limit)
        add(0,cfg=cfg);add(2,100,cfg=cfg)
        for k in range(limit+1):
            add(2,100,0xffffffff,(0,2,3,4,5,77)[k%6],cfg)
        add(2,100,50000,1,cfg)
        add(2,50,0xffffffff,0,cfg)
        add(2,100,75000,1,cfg)
        add(1,cfg=cfg);add(2,100,0,0,cfg)
    for field in range(5):
        for bad in (0,65536 if field==4 else 1001,0xffffffff):
            cfg=list(DEFAULT);cfg[field]=bad
            add(0,cfg=cfg);add(2,186,0,0,cfg)
    add(0)
    deterministic=len(rows)
    for cfg in CONFIGS:
        seed=0x0B170017
        for _ in range(32):
            add(0,cfg=cfg)
            for _ in range(256):
                words=[]
                for _ in range(4):
                    seed=(1664525*seed+1013904223)&0xffffffff
                    words.append(seed)
                action=words[0]%16
                action=action if action<2 else 2
                selector=words[3]%8
                state=(1,0,2,3,4,5,77,1)[selector]
                value=words[2] if selector!=7 else words[2]%185001
                add(action,words[1]%187,value,state,cfg)
    return rows, deterministic

def oracle(program, evidence):
    rows, deterministic=actions()
    inputs="".join(" ".join(map(str,r))+"\n" for r in rows)
    ref=Referencia()
    expected=[ref.step(row) for row in rows]
    result=subprocess.run([program],input=inputs,text=True,capture_output=True,check=False)
    (evidence/"oracle-input.txt").write_text(inputs)
    (evidence/"oracle-actual.txt").write_text(result.stdout)
    (evidence/"oracle-stderr.log").write_text(result.stderr)
    (evidence/"oracle-expected.txt").write_text("".join(" ".join(map(str,x))+"\n" for x in expected))
    assert result.returncode==0, f"emisor exit{result.returncode}: {result.stderr}"
    actual=[tuple(map(int,line.split())) for line in result.stdout.splitlines()]
    assert len(actual)==len(expected),(len(actual),len(expected))
    for k,(want,got) in enumerate(zip(expected,actual)):
        assert got==want, f"S9 paso{k} entrada={rows[k]} esperado={want} obtenido={got}"
    summary=dict(exact_steps=len(rows),deterministic_steps=deterministic,
                 random_steps=49152,sequence_initializations=192)
    print("PASS S9",summary)
    return summary

def plant(program, evidence):
    ref=Referencia()
    process=subprocess.Popen([program],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,bufsize=1)
    trace=[]
    def send(row):
        process.stdin.write(" ".join(map(str,row))+"\n");process.stdin.flush()
        got=tuple(map(int,process.stdout.readline().split()))
        want=ref.step(row)
        assert got==want,("S8",row,want,got)
        return got
    send((0,0,0,1,*DEFAULT))
    x=0;previous_u=0
    try:
        for k in range(200):
            goal=100 if k<100 else 185
            measured=2000*((x+1000)//2000)
            got=send((2,goal,measured,1,*DEFAULT))
            u=got[1]
            x=(3*x+200*previous_u+2)//4
            previous_u=u
            trace.append(dict(k=k,goal=goal,measured=measured,u=u,x_next=x,integral_num=got[2]))
    finally:
        process.stdin.close()
        stdout_tail=process.stdout.read()
        errors=process.stderr.read()
        code=process.wait()
        (evidence/"plant-stderr.log").write_text(errors)
    assert code==0 and not stdout_tail,(code,errors,stdout_tail)
    (evidence/"plant-trace.json").write_text(json.dumps(trace,indent=2)+"\n")
    metrics=[]
    for start,goal,amplitude in ((0,100,100),(100,185,85)):
        values=[p["x_next"] for p in trace[start:start+100]]
        peak=max(values);excess=max(0,peak-goal*1000)
        settling=next((j+1 for j in range(100) if all(abs(v-goal*1000)<=2000 for v in values[j:])),None)
        assert 10*excess<=amplitude*1000,("S8 sobrepico",goal,peak)
        assert settling is not None and settling<=20,("S8 establecimiento",goal,settling)
        metrics.append(dict(goal_rpm=goal,peak_milli_rpm=peak,overshoot_milli_rpm=excess,
                            settling_steps=settling,horizon_steps=100))
    print("PASS S8 planta de prueba (sin sintonia real):",metrics)
    return metrics

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--programa",required=True)
    parser.add_argument("--evidencia",type=Path,required=True)
    parser.add_argument("--solo-oraculo",action="store_true")
    args=parser.parse_args()
    args.evidencia.mkdir(parents=True,exist_ok=True)
    summary=oracle(str(Path(args.programa).resolve()),args.evidencia)
    if not args.solo_oraculo:
        summary["plant_metrics"]=plant(str(Path(args.programa).resolve()),args.evidencia)
    (args.evidencia/"results.json").write_text(json.dumps(summary,indent=2)+"\n")
if __name__=="__main__":
    main()

"""Spike C7: altissimo approximation = plain cylinder (no lattice, a = 7.5 mm, unflanged end), length solved so
f1 = target * 2^(25/1200); reed resonance raised for high notes: fr = max(1500, k * f_target). Does it speak
in R1 near the target for written C#6 (85) .. C7 (96)?"""
import numpy as np, json
from tmm_spike import zin, peaks
from reed_spike import modal_fit, run
from spike_c6 import f0spec
A=7.5e-3; f=np.linspace(20,9000,89801); res={}
def solve(t):
    lo,hi=0.01,0.5
    for _ in range(40):
        mid=(lo+hi)/2; p=peaks(f, zin(f,A,[('tube',mid)]),1); f1=p[0][0] if p else 1e9
        lo,hi=(mid,hi) if f1>t else (lo,mid)
    return mid
for w in (85,88,91,94,96):
    tgt=440*2**((w-2-69)/12); L=solve(tgt*2**(25/1200)); s,C=modal_fit(f,zin(f,A,[('tube',L)]),fmax=9000,nmax=8)
    row={'L_mm':round(L*1000,1),'nmodes':len(s)}
    for k in (1.0,1.5,2.0):
        fr=max(1500.0,k*tgt)
        try:
            x=run(s,C,0.6,tau=1e-2,T=0.25,qr=0.7,fr=fr); ff=f0spec(*x); row[f'k{k}']=round(1200*np.log2(ff/tgt),1)
        except Exception as e: row[f'k{k}']=str(e)[:40]
    res[w]=row; print(w,row,flush=True)
json.dump(res,open('spike_c7_result.json','w'),indent=1)

"""Spike C8: upper clarion (written 71..84 = geometry of written-19 + register hole open, mode-1 bw x10,
qr 0.7, fr = max(1500, 2 f_target)): regime and tuning vs the target note."""
import numpy as np, json
from reed_spike import solve_L, modal_fit, zgeom, run
from spike_c6 import f0spec
f=np.linspace(20,4500,44801); res={}
for w in (71,74,77,80,84):
    g_w=w-19; t1=440*2**((g_w-2-69)/12); tgt=440*2**((w-2-69)/12)
    L=solve_L(t1*2**(25/1200)); so,Co=modal_fit(f,zgeom(f,L,True),fmax=4000)
    so[0]=so[0].real*10+1j*so[0].imag; row={}
    for g in (0.5,0.65):
        for tau in (3e-3,2e-2):
            x=run(so,Co,g,tau=tau,T=0.3,qr=0.7,fr=max(1500,2*tgt)); ff=f0spec(*x)
            row[f'g{g}_t{tau}']=round(1200*np.log2(ff/tgt),1)
    res[w]=row; print(w,row,flush=True)
json.dump(res,open('spike_c8_result.json','w'),indent=1)

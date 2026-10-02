"""Spike C3: which emulation of register-hole nonlinear losses makes the linear modal model play the
second register (R2 = 3x the closed-hole note) reliably with the register hole open?"""
import numpy as np, json
from reed_spike import solve_L, modal_fit, zgeom, run, f0
f=np.linspace(20,3500,34801); out={}
for name,tgt in (('F3',174.61),('C4',261.63)):
    L=solve_L(tgt*2**(25/1200)); so,Co=modal_fit(f,zgeom(f,L,True))
    for label,(bw,cs) in {'bw10':(10,1.0),'bw25':(25,1.0),'c0.3':(1,0.3),'bw10_c0.5':(10,0.5)}.items():
        s2=so.copy(); C2=Co.copy(); s2[0]=s2[0].real*bw+1j*s2[0].imag; C2[0]*=cs
        cnt={'r1o':0,'r2':0,'other':0,'eq':0}; f2=[]
        for g in (0.45,0.55,0.65,0.75):
            for tau in (1e-3,1e-2,3e-2):
                sig,Fs=run(s2,C2,g,tau=tau,T=0.4); ff=f0(sig,Fs)
                if ff==0: r='eq'
                elif abs(ff/(3*tgt)-1)<0.06: r='r2'; f2.append(ff)
                elif abs(ff/(so[0].imag/2/np.pi)-1)<0.1: r='r1o'
                else: r='other'
                cnt[r]+=1
        cents=[round(1200*np.log2(x/(3*tgt)),1) for x in f2]
        out[f'{name}_{label}']={'counts':cnt,'r2_cents_vs_twelfth':cents[:4]}
        print(name,label,out[f'{name}_{label}'],flush=True)
json.dump(out,open('clarion_map_result.json','w'),indent=1)

"""Spike C9: replay T-009 exactly as frozen (fixture modes, rounded to 6 significant digits)."""
import json, numpy as np
from reed_spike import run
from spike_c6 import f0spec
d=json.load(open('/home/claude/clar/tests/fixtures/resonators_valid.json'))
e=[x for x in d['entries'] if x['vent']=='register'][0]
s=np.array([m['re_s']+1j*m['im_s'] for m in e['modes']]); C=np.array([m['re_c']+1j*m['im_c'] for m in e['modes']])
s2=s.copy(); s2[0]=s2[0].real*10+1j*s2[0].imag
res={'linear':[], 'emulated':[]}
for g in (0.5,0.65):
    for tau in (3e-3,1e-2,3e-2):
        for k,ss in (('linear',s),('emulated',s2)):
            x,Fs=run(ss,C,g,zeta=0.4,tau=tau,T=0.4,fr=1500.0,qr=0.7,Kc=100.0,eta=0.01)
            y=x[len(x)//2:]; ff=f0spec(y,Fs) if np.std(y)>1e-3 else 0
            res[k].append(round(ff/174.61,3))
print(res); json.dump(res,open('t009_check_result.json','w'),indent=1)

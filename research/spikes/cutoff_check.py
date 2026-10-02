"""Spike C10: Petersen et al. 2020 Table 1 resonators (a 6.5 mm, 10 open holes): first peak ~185 Hz for all
three; cutoff detected as the first impedance peak lower than 30 % of its predecessor."""
import numpy as np
from tmm_spike import zin, peaks
rows={1000:(398.8,9.8,6.5,2.5,16.3),1500:(417.0,9.8,6.5,4.0,16.3),2000:(426.0,9.8,6.5,5.8,16.3)}
f=np.linspace(20,5000,49801)
for fc,(L,h,a,b,l) in rows.items():
    a,b,h,l,L=a*1e-3,b*1e-3,h*1e-3,l*1e-3,L*1e-3
    seg=[('tube',L+l)]
    for i in range(10): seg+= [('hole',b,h,True),('tube',2*l if i<9 else l)]
    p=peaks(f,zin(f,a,seg),40)
    det=next(p[i][0] for i in range(1,len(p)) if p[i][1]<0.3*p[i-1][1])
    print(fc,'f1',p[0][0],'detected cutoff',det, round(det/fc,3))
# output 2026-10-02: 1000 f1 185.2 cutoff 1105.3 (1.105); 1500 f1 184.8 cutoff 1511.5 (1.008); 2000 f1 184.6 cutoff 1897.8 (0.949)

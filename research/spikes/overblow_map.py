"""Spike C5: closed-hole overblowing (no register key). Which control settings give R2^(c) (twelfth) or a squeak?
Basis for the Overblow knob (D-006) and the 'Overblown fingering' mode (D-007)."""
import numpy as np, json, itertools
from reed_spike import solve_L, modal_fit, zgeom, run
from reed_map import label, sc, Cc
out={}
for g,t,qr,z in itertools.product((0.6,0.85,1.0),(1e-3,1e-2),(0.7,0.3),(0.4,0.2)):
    out[f'g{g}_tau{t}_qr{qr}_z{z}']=label(*run(sc,Cc,g,zeta=z,tau=t,T=0.3,qr=qr))
print(out)
# 'voicing': suppress mode 1 (residue x0.3) as in the sax harmonic mode
s2=sc.copy(); C2=Cc.copy(); C2[0]*=0.3
v={f'g{g}_tau{t}':label(*run(s2,C2,g,tau=t,T=0.3,qr=0.7)) for g in (0.6,0.8) for t in (1e-3,1e-2)}
print('mode1 residue x0.3:',v)
s3=sc.copy(); s3[0]=s3[0].real*10+1j*s3[0].imag
v2={f'g{g}_tau{t}':label(*run(s3,Cc,g,tau=t,T=0.3,qr=0.7)) for g in (0.6,0.8) for t in (1e-3,1e-2)}
print('mode1 bw x10:',v2)
json.dump({'grid':out,'mode1_residue_0.3':v,'mode1_bw_x10':v2},open('overblow_map_result.json','w'),indent=1)

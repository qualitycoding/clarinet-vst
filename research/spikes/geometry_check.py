"""Spike C11: D-009 virtual clarinet — length solve, odd series ratio, register-hole effect, per written note."""
import numpy as np
from reed_spike import solve_L, zgeom
from tmm_spike import peaks
f=np.linspace(40,2500,49201)
for w in (52,55,58,60,65,70):
    t=440*2**((w-2-69)/12)*2**(25/1200); L=solve_L(t)
    pc=peaks(f,zgeom(f,L),2)
    line=f"w{w} L={L:.4f} f1={pc[0][0]} tgt={t:.2f} ratio={pc[1][0]/pc[0][0]:.3f}"
    if w<=65:
        po=peaks(f,zgeom(f,L,True),2); line+=f" open f1={po[0][0]} ({po[0][1]} vs {pc[0][1]}) f2 shift={po[1][0]/pc[1][0]:.4f}"
    print(line)
# output 2026-10-02:
# w52 L=0.5267 f1=148.9 tgt=148.97 ratio=3.019 open f1=211.2 (16.3 vs 37.0) f2 shift=1.0120
# w55 L=0.4371 f1=177.1 tgt=177.15 ratio=3.015 open f1=238.7 (20.3 vs 40.2) f2 shift=1.0022
# w58 L=0.3616 f1=210.6 tgt=210.67 ratio=3.011 open f1=269.0 (25.1 vs 43.6) f2 shift=1.0002
# w60 L=0.3180 f1=236.4 tgt=236.47 ratio=3.006 open f1=291.0 (28.7 vs 45.9) f2 shift=1.0031
# w65 L=0.2285 f1=315.7 tgt=315.65 ratio=2.981 open f1=355.4 (39.6 vs 52.2) f2 shift=1.0188
# w70 L=0.1612 f1=421.4 tgt=421.35 ratio=2.916

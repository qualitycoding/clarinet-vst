"""Spike C4: reed resonance/damping vs regime selection. Open register hole (clarion) and closed (chalumeau).
Regime labels from the spectrum peak: R1 (closed f), R2 (3x), R1o (open-hole first peak), SQ (>= 900 Hz, 'squeak')."""
import numpy as np, json, itertools
from reed_spike import solve_L, modal_fit, zgeom, run
f=np.linspace(20,3500,34801); tgt=174.61
L=solve_L(tgt*2**(25/1200)); sc,Cc=modal_fit(f,zgeom(f,L)); so,Co=modal_fit(f,zgeom(f,L,True))
def label(sig,Fs):
    s=sig[len(sig)//2:]; s=s-s.mean()
    if np.std(s)<1e-3: return 'EQ'
    S=np.abs(np.fft.rfft(s*np.hanning(len(s)),4*len(s))); fr=np.fft.rfftfreq(4*len(s),1/Fs)
    # fundamental = lowest spectral peak above -20 dB of max
    m=S.max(); cand=[i for i in range(1,len(S)-1) if S[i]>0.1*m and S[i]>=S[i-1] and S[i]>=S[i+1] and fr[i]>60]
    p=fr[cand[0]]
    if abs(p/tgt-1)<0.06: return 'R1'
    if abs(p/(3*tgt)-1)<0.06: return 'R2'
    if abs(p/(so[0].imag/2/np.pi)-1)<0.08: return 'R1o'
    if p>900: return 'SQ'
    return 'X%.0f'%p
if __name__=='__main__':
    out={}
    if __name__!="__main__": pass
    for fr_,qr,bw in itertools.product((1500.0,2200.0),(0.4,0.7,1.0),(1,10)):
        s2=so.copy(); s2[0]=s2[0].real*bw+1j*s2[0].imag
        lo=[label(*run(s2,Co,g,tau=t,T=0.3,fr=fr_,qr=qr)) for g in (0.5,0.65) for t in (3e-3,1e-2,3e-2)]
        lc=[label(*run(sc,Cc,g,tau=t,T=0.3,fr=fr_,qr=qr)) for g in (0.5,0.65) for t in (3e-3,1e-2,3e-2)] if bw==1 else None
        k=f'fr{fr_:.0f}_qr{qr}_bw{bw}'; out[k]={'open':lo,'closed':lc}; print(k,out[k],flush=True)
    json.dump(out,open('reed_map_result.json','w'),indent=1)

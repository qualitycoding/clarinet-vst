"""Spike C6: (a) tuning of R2 closed-hole (overblown fingering, mode-1 bw x10) vs register-hole R2;
(b) extreme overblow (gamma 1.0, tau 1 ms, qr 0.25) behaviour; (c) altissimo 'virtual short tube' (first
resonance solved at the target) speaks R1 for written C#6 and C7; (d) throat Bb4 written."""
import numpy as np, json
from reed_spike import solve_L, modal_fit, zgeom, run
from reed_map import label
def f0spec(sig,Fs):
    s=sig[len(sig)//2:]; s=s-s.mean(); S=np.abs(np.fft.rfft(s*np.hanning(len(s)),8*len(s))); fr=np.fft.rfftfreq(8*len(s),1/Fs)
    m=S.max(); c=[i for i in range(1,len(S)-1) if S[i]>0.1*m and S[i]>=S[i-1] and S[i]>=S[i+1] and fr[i]>60]
    i=c[0]; a,b,cc=np.log(S[i-1]),np.log(S[i]),np.log(S[i+1]); d=0.5*(a-cc)/(a-2*b+cc); return (fr[i]+d*(fr[1]-fr[0]))
if __name__=='__main__':
    f=np.linspace(20,3500,34801); res={}
    for name,w in (('G3',55),('C4',60)):
        tgt=440*2**((w-2-69)/12); L=solve_L(tgt*2**(25/1200))
        sc,Cc=modal_fit(f,zgeom(f,L)); so,Co=modal_fit(f,zgeom(f,L,True))
        s1=sc.copy(); s1[0]=s1[0].real*10+1j*s1[0].imag; s2=so.copy(); s2[0]=s2[0].real*10+1j*s2[0].imag
        a=run(s1,Cc,0.65,tau=3e-3,T=0.4,qr=0.7); b=run(s2,Co,0.65,tau=3e-3,T=0.4,qr=0.7)
        res[name]={'closed_overblown_cents_vs_12th':round(1200*np.log2(f0spec(*a)/(3*tgt)),1),'label_closed':label(*a),
                   'reghole_cents_vs_12th':round(1200*np.log2(f0spec(*b)/(3*tgt)),1),'label_open':label(*b),
                   'extreme_closed':label(*run(s1,Cc,1.0,tau=1e-3,T=0.3,qr=0.25)),'extreme_open':label(*run(s2,Co,1.0,tau=1e-3,T=0.3,qr=0.25))}
        print(name,res[name],flush=True)
    for name,w in (('Bb4_throat',70),('C#6_alt',85),('C7_alt',96)):
        tgt=440*2**((w-2-69)/12); L=solve_L(tgt*2**(25/1200)); s,C=modal_fit(f,zgeom(f,L))
        out={}
        for g in (0.5,0.7):
            x=run(s,C,g,tau=1e-2,T=0.3,qr=0.7); ff=f0spec(*x); out[f'g{g}']=round(1200*np.log2(ff/tgt),1)
        res[name]={'L_m':round(L,4),'nmodes':len(s),'cents_vs_target':out}; print(name,res[name],flush=True)
    json.dump(res,open('spike_c6_result.json','w'),indent=1)

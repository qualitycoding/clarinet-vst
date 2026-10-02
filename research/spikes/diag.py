import numpy as np
from reed_spike import solve_L, modal_fit, zgeom, run, f0, harm
f=np.linspace(20,3500,34801); tgt=174.61
L=solve_L(tgt*2**(25/1200)); so,Co=modal_fit(f,zgeom(f,L,True))
print('open peaks',[round(x.imag/2/np.pi,1) for x in so], 'bw', [round(-x.real/np.pi,1) for x in so][:3])
s2=so.copy(); s2[0]=s2[0].real*10+1j*s2[0].imag
for g in (0.45,0.55,0.65,0.75):
    for tau in (1e-3,1e-2,3e-2):
        sig,Fs=run(s2,Co,g,tau=tau,T=0.4); ff=f0(sig,Fs)
        s=sig[len(sig)//2:]; S=np.abs(np.fft.rfft((s-s.mean())*np.hanning(len(s)))); fr=np.fft.rfftfreq(len(s),1/Fs)
        top=fr[np.argsort(S)[-3:]]
        print(g,tau,round(ff,1),'top bins',np.round(top,0), 'rms',round(float(np.std(s)),3))

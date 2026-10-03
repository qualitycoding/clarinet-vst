"""Spike C2: clarinet-like modal synthesis. Reed model = Petersen et al. 2020 eqs (7)-(9) (same form as
Colinot 2021, used by the sax plan), params Table 2 (fr 1500 Hz, qr 0.4, Kc 100, eta 0.01; beta omitted),
controls Table 3 (gamma 0.5 piano / 0.7 forte, zeta 0.4). Resonator: a=7.5 mm cylinder, upstream length L,
lattice of 5 open holes with the Petersen fc=1.5 kHz cell scaled by 7.5/6.5 in b (eq. 1 => same fc),
optional register hole (Debut 2005: 140 mm, r 1.55 mm, h 12.5 mm)."""
import numpy as np, json, sys
from tmm_spike import zin, peaks, C0
A, B, H, LL = 7.5e-3, 4.0e-3*7.5/6.5, 9.8e-3, 16.3e-3
def geom(L, reg_open=None, nlat=5, vent=None):
    seg=[]
    if reg_open is None: seg.append(('tube', L+LL))
    else:
        seg += [('tube',0.140), ('hole',1.55e-3,12.5e-3,reg_open), ('tube', L+LL-0.140)]
    for i in range(nlat):
        seg.append(('hole',B,H,True)); seg.append(('tube', 2*LL if i<nlat-1 else LL))
    return seg
def zgeom(f, L, reg=None): return zin(f, A, geom(L, reg))
def solve_L(target, reg=None):
    f=np.linspace(40.0, 2000.0, 39201); lo, hi = 0.02, 0.9
    for _ in range(40):
        mid=(lo+hi)/2; p=peaks(f, zgeom(f, mid, reg), 1)
        f1 = p[0][0] if p else 1e9
        if f1 > target: lo=mid
        else: hi=mid
    return mid
def modal_fit(f, z, fmax=3000.0, nmax=16):
    m=np.abs(z); idx=[i for i in range(1,len(m)-1) if m[i]>m[i-1] and m[i]>=m[i+1] and f[i]<fmax][:nmax]
    poles=[]
    for i in idx:
        half=m[i]/np.sqrt(2); j=i
        while j>0 and m[j]>half: j-=1
        k=i
        while k<len(m)-1 and m[k]>half: k+=1
        bw=max(f[k]-f[j], 1.0); poles.append(-np.pi*bw + 2j*np.pi*f[i])
    s=np.array(poles); w=2j*np.pi*f
    Mr = np.stack([1/(w-sn)+1/(w-np.conj(sn)) for sn in s],1)
    Mi = np.stack([1j/(w-sn)-1j/(w-np.conj(sn)) for sn in s],1)
    M=np.concatenate([Mr,Mi],1); sel=f<fmax*1.1
    Mrr=np.concatenate([M[sel].real, M[sel].imag]); zz=np.concatenate([z[sel].real, z[sel].imag])
    x,*_=np.linalg.lstsq(Mrr, zz, rcond=None); n=len(s)
    return s, x[:n]+1j*x[n:]
def run(s, C, gf, zeta=0.4, tau=0.01, T=0.5, Fs=176400, fr=1500.0, qr=0.4, Kc=100.0, eta=0.01):
    dt=1/Fs; N=int(T*Fs); E=np.exp(s*dt); G=C*(E-1)/s; wr=2*np.pi*fr
    pn=np.zeros(len(s),complex); x=0.0; xp=0.0; out=np.empty(N)
    for k in range(N):
        t=k*dt; g=gf/2*(1+np.tanh((t-5*tau)/tau)); p=2*np.sum(pn.real)
        o=x+1; mn=(o-np.sqrt(o*o+eta))/2; Fc=Kc*mn*mn
        xn=(dt*dt*wr*wr*(p-g+Fc-x)+2*x-xp+0.5*qr*wr*dt*xp)/(1+0.5*qr*wr*dt); xp,x=x,xn
        o=x+1; mx=(o+np.sqrt(o*o+eta))/2; d=g-p
        u=zeta*mx*np.sign(d)*np.sqrt(np.sqrt(d*d+eta)); pn=E*pn+G*u; out[k]=p
    return out, Fs
def f0(sig,Fs):
    s=sig[len(sig)//2:]; s=s-s.mean()
    if np.max(np.abs(s))<1e-3: return 0.0
    ac=np.correlate(s[::4],s[::4],'full')[len(s[::4])-1:]; ac/=ac[0]; fs=Fs/4
    lo=int(fs/2500); hi=int(fs/90); i=lo+np.argmax(ac[lo:hi]); return fs/i
def harm(sig,Fs,f,n=6):
    s=sig[len(sig)//2:]; s=(s-s.mean())*np.hanning(len(s)); S=np.abs(np.fft.rfft(s,4*len(s))); fr=np.fft.rfftfreq(4*len(s),1/Fs)
    L=[20*np.log10(max(S[(fr>h*f*0.97)&(fr<h*f*1.03)].max(),1e-12)) for h in range(1,n+1)]
    return [round(v-max(L),1) for v in L]
if __name__=='__main__':
    f=np.linspace(20,3500,34801); res={}
    tgt=174.61  # written G3 -> concert F3
    L=solve_L(tgt*2**(25/1200)); s,C=modal_fit(f,zgeom(f,L)); res['L_F3']=round(L,4); res['nmodes']=len(s)
    res['peaks_F3']=[round(x.imag/2/np.pi,1) for x in s][:8]
    for g in (0.5,0.7):
        sig,Fs=run(s,C,g); ff=f0(sig,Fs); res[f'F3_g{g}']={'f0':round(ff,1),'harm_dB':harm(sig,Fs,ff)}
        print(g,res[f'F3_g{g}'],flush=True)
    # clarion: same geometry with register hole open -> expect R2 near 3*f
    so,Co=modal_fit(f,zgeom(f,L,True)); res['peaks_F3_reg_open']=[round(x.imag/2/np.pi,1) for x in so][:6]
    cnt={'r1':0,'r2':0,'other':0,'eq':0}
    for g in (0.45,0.55,0.65,0.75):
        for tau in (1e-3,1e-2,3e-2):
            sig,Fs=run(so,Co,g,tau=tau,T=0.4); ff=f0(sig,Fs)
            r='eq' if ff==0 else ('r1' if abs(ff/tgt-1)<0.08 else ('r2' if abs(ff/(3*tgt)-1)<0.06 else 'other')); cnt[r]+=1
    res['clarion_linear']=cnt; print('clarion linear',cnt,flush=True)
    # emulate localized nonlinear losses: extra damping on mode 1 (x4 bandwidth)
    so2=so.copy(); so2[0]=so2[0].real*4+1j*so2[0].imag
    cnt={'r1':0,'r2':0,'other':0,'eq':0}
    for g in (0.45,0.55,0.65,0.75):
        for tau in (1e-3,1e-2,3e-2):
            sig,Fs=run(so2,Co,g,tau=tau,T=0.4); ff=f0(sig,Fs)
            r='eq' if ff==0 else ('r1' if abs(ff/tgt-1)<0.08 else ('r2' if abs(ff/(3*tgt)-1)<0.06 else 'other')); cnt[r]+=1
    res['clarion_mode1_damped_x4']=cnt; print('clarion damped',cnt,flush=True)
    json.dump(res,open('reed_spike_result.json','w'),indent=1); print(json.dumps(res))

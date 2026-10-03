"""Spike C1: plane-wave TMM of clarinet-like cylinders with side holes (lossy, Chaigne-Kergomard 5.5 losses
as quoted in Szwarcberg et al. 2025 sec 3.2), unflanged radiation, simple open/closed side-hole shunts.
Checks (a) Petersen 2020 Table 1 fc=1.5 kHz resonator, all holes open: f1 ~ 185 Hz;
(b) lattice cutoff ~ 1.5 kHz visible (peak spacing/height change); (c) register hole effect."""
import numpy as np
C0, RHO, LV = 343.37, 1.2041, 4e-8
def gamma(f, R):
    s = 2j*np.pi*f
    return s/C0 + 1.044/R*np.sqrt(2*LV/C0)*np.sqrt(s) + 1.080*LV/R**2
def tube(f, R, L):
    G = gamma(f, R); Zc = RHO*C0/(np.pi*R**2)
    ch, sh = np.cosh(G*L), np.sinh(G*L)
    return np.array([[ch, Zc*sh],[sh/Zc, ch]])
def zrad(f, R):
    k = 2*np.pi*f/C0; Zc = RHO*C0/(np.pi*R**2)
    return Zc*(0.25*(k*R)**2 + 1j*0.6133*k*R)
def hole_shunt(f, b, h, open_):
    G = gamma(f, b); Zh = RHO*C0/(np.pi*b**2)
    if open_:
        t = h + 0.6133*b + 0.6*b   # outer radiation + inner matching correction (approx.)
        return Zh*(np.tanh(G*t) + 0.25*(2*np.pi*f/C0*b)**2)
    return Zh/np.tanh(G*h)
def zin(f, R, segments, end_open=True):
    """segments: list of ('tube',L) or ('hole',b,h,open). Computes from the far end back to the input."""
    Z = zrad(f, R) if end_open else np.full_like(f, 1e12, dtype=complex)
    for seg in reversed(segments):
        if seg[0]=='tube':
            M = tube(f, R, seg[1]); Z = (M[0,0]*Z + M[0,1])/(M[1,0]*Z + M[1,1])
        else:
            Zs = hole_shunt(f, seg[1], seg[2], seg[3]); Z = 1/(1/Z + 1/Zs)
    return Z/(RHO*C0/(np.pi*R**2))
def peaks(f, z, n=30):
    m=np.abs(z); idx=[i for i in range(1,len(m)-1) if m[i]>m[i-1] and m[i]>=m[i+1]]
    return [(round(f[i],1), round(m[i],1)) for i in idx[:n]]
def petersen(nopen=10, nclosed=0):
    a,b,h,l,L = 6.5e-3,4.0e-3,9.8e-3,16.3e-3,417.0e-3
    seg=[('tube',L+l)]
    for i in range(10):
        seg.append(('hole',b,h,i>=nclosed)); seg.append(('tube',2*l if i<9 else l))
    return a,seg
if __name__=='__main__':
    f=np.linspace(20,5000,49801)
    a,seg=petersen()
    z=zin(f,a,seg); p=peaks(f,z)
    print('Petersen fc1500 all open: peaks',p[:12])
    a,seg=petersen(nclosed=5)
    print('5 closed:',peaks(f,zin(f,a,seg))[:8])
    # plain closed-open cylinder check
    z=zin(f,7.5e-3,[('tube',0.5)]); print('cyl 0.5m:',peaks(f,z)[:3],' expect ~',[ (2*n-1)*C0/(4*(0.5+0.6133*7.5e-3)) for n in (1,2,3)])

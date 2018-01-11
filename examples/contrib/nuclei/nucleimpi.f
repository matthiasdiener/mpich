      program main
C                                                                       version
C  ***********************************************************
C  *        16O    C. E. A   MPI  Version                    *
C  ***********************************************************
C
c
c     N U C L E I        P R O G R A M                                     10   
c
c     6/3/89 - add subrho call to get rho(r12) errors
c     8/1/89 - cards for automatic alpha, rho conversions
c     10/8/89 - use macros, /option/ stuff
c     10/27/89 - fijk
c     11/1/89 - patches for reading version 6 files
c     12/14/89 - uijk first or last
c     3/12/90 - write/read on savunt
c     3/15/90 - twof6
c     3/19/90 - unit 9 debugging & trace output                            20   
c     3/21/90 - savunt output for a difference calculation
c     3/29/90 - twof8
c     5/6/90 - remove the bars, control of log output; version 10
c     5/17/90 - disconnected sums
c     7/26/90 - rho(k)
c     9/4/90 - save rho(k) stuff
c     9/12/90 - wtfac2 added
c     9/25/90 - fc(k) added
c     10/22/90 - remove dets, mixed rho
c     10/31/90 - pion energy added                                         30   
c     11/29/90 - read 10 values of prmrad for vso
c     12/6/90 - read prmrad(11-20) for ityprd=40,46
c     12/16/90 - 12c changes
c     12/26/90 - add tracsw to twof6 call
c     12/27/90 - changes for merging det and nodet subroutines
c     3/11/91 - changes for u(5&6) after u(1-4)
c     4/1/91 - use open statements
c     1/15/92 - write save unit every cycle in differece calc (India)
c     2/19/92 - generalized cluster expansion
c     3/10/92 - major rewrite of the file handling                         40   
c     5/5/92 - add save stuff to setrhk call
c     5/13/92 - extend grid ranges for rho(k)
c     6/18/92 - complex determ version
c     8/11/92 - Version 21 - WTLIM replaces WTPOW3, 4 in determ version
c     9/8/92 - define threesw, foursw, discsw; cc4 on setfou
c     9/22/92 - fix up accumu calls for recovery from a crash
c     9/26/92 - close/open walkun, etc, for recovery from failuers
c     9/28/92 - code for eliminating a nucleon
c     10/5/92 - doener becomes doke finally; dov2, f8sw
c     10/22/92 - quasi-hole stuff                                          50   
c     3/22/93 - (e,e'p) stuff
c     6/11/93 - Version 22 - surface waves
c     6/18/93 - Version 23 - more surface wave parameters
c     7/12/93 - rearrainge for parallel; add CP4 stuff
c     8/10/93 - Version 24 - remove i56 from ijordr;
c               ijordr in output file
c     8/27/93 - Version 25 - f3c(i,j,k)
c     9/22/93 - Version 26 - new Vijk, Uijk params
c
      implicit real*8 ( a-h, o-z )                                      implicit
c
      logical logsw, backsw
c
c    the random walk is optionally written on unit walkun.  it may
c    be restarted from walkun.
c    additional stuff is saved on saveun
c    dbugun is used to write specified quanitities for each configuratio
c
      integer walkun, saveun, dbugun
c                                                                          70   
      parameter ( walkun = 12 , saveun = 8 , dbugun = 9 )
c
c
      parameter ( nmpu = 4 ,  nmpd = 4 ,  nmnu = 4 ,  nmnd = 4 ,        numpar
     &   numpar_walk = 16 ,  numpar_all = 16 ,
     2   maxl = 1 , numrad = 2 , numstate = 8 , numphis = 8 ,
     &   numdets = 4 , maxparindet = 4 ,  lendets = 4 ,
     &   nmtype = 4 ,  nmexcg = 4 ,
     &   nm1cl = 1 ,  nm2cl = 4 ,  nm3cl = 5 ,   nm4cl = 11 ,
     &   mxdis2 = 1 ,  nm12cl = 1 ,  nm13cl = 1 ,  nm22cl = 1 ,
     &   nm1en = 4 ,  nm2en = 4 ,  nm3en = 4 ,   nm4en = 4 ,
     &   nm12en = 1 ,   nm13en = 1 ,   nm22en = 1 ,
     &   mx1pts = 16, mx2pts = 33, mx3pts = 257, mx4pts = 385,
     &   ln4wrk = 450 + 1  ,  nmstrip = ln4wrk-1 ,
     &   mx12pts = 1 ,  mx13pts = 1 ,  mx22pts = 1 )
      logical peqivn
      parameter ( peqivn = .true. )
      parameter ( nmrh1 = 3 ,   nmrh2 = 4+5 ,   maxrho = 51 )
      parameter ( mxgaus = 3 , ns1or2 = 2 , mxks = 1 )
      parameter ( mxspl1 = 201 )
      parameter ( numprot = nmpu+nmpd , numneut= nmnu+nmnd ,            genparam
     &  numpar = numprot+numneut ,
     4  matdim = maxparindet+1 ,
     2  npardm = numpar_all+1 ,  lnpar = numpar+1 , mxrorc = 2  )
      parameter (  nmpair_all = (numpar_all*(numpar_all-1))/2,
     &  nmpair_walk = (numpar_walk*(numpar_walk-1))/2 )
      parameter ( nmpair = (numpar*(numpar-1))/2 ,                      opsparam
     3   lnpair = nmpair+1 ,
     4   nmtrip = (nmpair*(numpar-2))/3 , lntrip = nmtrip+1 ,
     5   nmquad = (nmtrip*(numpar-3))/4 , lnquad = nmquad + 1 ,
     1   npairprot = (numprot*(numprot-1))/2 ,
     2   ntripprot = (npairprot*(numprot-2))/3 )
c                                                                       /onecom/
c     RADNODES, RADL, RADJ - nlj of each radial wavefunction;
C       RADJ is 2*j.  RADJ = -1 means no L.S is being used.
C   following define each "state" in PHIS
C     RADFUN - radial function to use, index to RADNODES, etc.
C     RADML - M value for Y(L,M)
C     RADMS - Spin indicator: 1 = up, 0 = down
C     RADFAC - Clebsch factor.
C   following define each single-particle wavefunction which is
C     a sum of the states defined above.
C     PHISTATE - index to the 1st element of RADFUN, etc.
C     PHINUM - number of states to add up.
C     PHILAST - index to the last element of RADFUN, etc.
C   following define each particle:
C     PARPHI - index to the PHISTATE, PHINUM array.
C     PARPORN - 0 = proton; 1 = neutron.
C     PARINOPER - .T. means the particle is an operator particle
c     PARWLKMAP - mapping array of where to particle positions read
c                 from walk file are to be stored.  Note extra last
c                 dummy element.
c     PARDET - determinant number (1 to NUMDETS)
c     PARINDET - index in the determ for the nucleon:
c           MATS(DETOFF(PARDET(i))+PARINDET(i))  is the row for i
C     IPAR(i) - index to the full (NUMPAR_ALL) particle array
C         for an operator particle (NUMPAR) index.
C     IIPAR(i)  index to the operator array for a particle in the
c         full array - 0 = not an operator nucleon
c     I1S(i) = i - list of indeices to t1s for the operator nucleons.
c         This is always just  1, 2, ..., numpar
c   following define the determinants ( 1 =< k =< NUMDETS )
c     NPARINDET(k) - number of nucleons in each determ = rank
c     DETPORN(k) - 0 = protons, 1 = neutrons
c     DETMS - 0 = spin down, 1 = spin up
c     DETPAR(j,k) - list of nucleons in the determ; values are to
c                   the full list of nucleons (numpar_all)
c     DETSTATE(j,k) - list of states (pointers to RADxxx) in each det
c     DETOFF(k) - (index-1) of first row of mats:
c               MATS(DETOFF(k)+i,j)   1 =< i,j =< NPARINDET(k)
c
      integer radnodes, radl, radj, radfun, radml, radms,
     1  phistate, phinum, philast, parphi, parporn, partyp, parwlkmap
      logical parinoper
      common /onecom/ radnodes(numrad), radl(numrad), radj(numrad),
     1  radfun(numstate), radml(numstate), radms(numstate),
     2  radfac(numstate),
     3  phistate(numphis), phinum(numphis), philast(numphis),
     3  parphi(numpar_all), parporn(numpar_all), parinoper(numpar_all),
     1  partyp(numpar_all), parwlkmap(numpar_walk+1),
     4  ipar(numpar), iipar(numpar_all), i1s(numpar)
      integer detporn, detms, detpar, detstate, pardet, detoff,
     &  parindet
      common /onecom/ nparindet(numdets), detporn(numdets),
     &  detms(numdets), detpar(maxparindet, numdets),
     &  detstate(maxparindet, numdets), pardet(numpar_all),
     &  detoff(numdets), parindet(numpar_all)
      common /onefloat/ clebdiv, ylmfac((maxl+1)**2),
     5  rmaxsq, u0scal, xdu0in, xdelu0, rmx0sq,
     6  radcub(4, numrad, mxspl1)
cnodt      parameter ( mx1pts = lnpar , mx2pts = lnpair ,               /classes
cnodt     &   mx3pts = lntrip , mx4pts = lnquad ,
cnodt     &   mx12pts = numpar*nmpair  ,  mx13pts = numpar*nmtrip  ,
cnodt     &   mx22pts = nmpair**2 )
      integer excgs
      integer*2 k1cls, k2cls, k3cls, k4cls, k12cls, k13cls, k22cls
      common /classes/  div1cl(nm1cl+1), div2cl(nm2cl+1),
     &    div3cl(nm3cl+1), div4cl(nm4cl+1),
     &    div12cl(nm12cl+1), div13cl(nm13cl+1), div22cl(nm22cl+1),
     &  cls1nm(nm1cl+1), cls2nm(nm2cl+1),
     &    cls3nm(nm3cl+1), cls4nm(nm4cl+1),
     &  rat1cl(nm1cl+1), rat2cl(nm2cl+1),
     &    rat3cl(nm3cl+1), rat4cl(nm4cl+1),
     &  div1pts(nm1cl+1), div2pts(nm2cl+1),
     &    div3pts(nm3cl+1), div4pts(nm4cl+1),
     &  excgs(nmtype, nmexcg),
     &  i1cls(nm1en, nm1cl), i2cls(2, nm2en, nm2cl),
     &    i3cls(3, nm3en, nm3cl), i4cls(4, nm4en, nm4cl ),
     &    i12cls(3, nm12en, nm12cl), i13cls(4, nm13en, nm13cl),
     &    i22cls(4, nm22en, nm22cl),
     &  nm1pts(nm1cl+1), nm2pts(nm2cl+1), nm3pts(nm3cl+1),
     &    nm4pts(nm4cl+1),
     &    nm12pts(nm12cl+1), nm13pts(nm13cl+1), nm22pts(nm22cl+1)
      common /classes/
     &  m21cl(2, nm2cl), m31cl(3, nm3cl), m32cl(3, nm3cl),
     &  m41cl(4, nm4cl), m42cl(6, nm4cl), m43cl(4, nm4cl),
     &  m312cl(3, nm3cl),
     &  m412cl(12, nm4cl), m413cl(4, nm4cl), m422cl(6, nm4cl),
     &  m121cl(nm12cl), m122cl(nm12cl),
     &  m131cl(nm13cl), m132cl(3,nm13cl), m133cl(nm13cl),
     &      m1312cl(3,nm13cl),
     &  m221cl(2,nm22cl), m222cl(2,nm22cl), m2212cl(2,nm22cl)
      common /classes/
     &  k1cls(mx1pts, nm1cl), k2cls(mx2pts, nm2cl),
     &    k3cls(mx3pts, nm3cl), k4cls(mx4pts, nm4cl),
     &    k12cls(mx12pts, nm12cl), k13cls(mx13pts, nm13cl),
     &    k22cls(mx22pts, nm22cl)
cray>singledata
c                                                                          80   
c     array of possitions of particles: xyzs(particle num,alpha)
c         alpha = 1, 2, 3 for x, y, z
c     array of functions of one particle:  fs(particle number,alpha)
c      alpha   contents
c        1     r**2
c        2     xx = delta for cubic splines
c
c     grdpsj(i,x-y-z) - 1/psij grad(i) psij
c     grdpsj(i,4) = 1/psij laplacian(i) psij
c         these are computed by kinet1.                                    90   
c
C                                                                       /funci/
c     BOSEDL(particle, x-y-z, +-) - Psi(bose shifted)/Psi(bose)
C     PHIS(particle, complex, nljmjml)  - every phi state for every
C                 position - equivalenced to MATS
C     PHINRM - PHIS/sqrt( sum(states) ); for determs this is all 1's
C     PHISQ(particle) - <phi(i)(ri) | phi(i)(ri)>
c     PHNRMS(particle) - 1/sqrt(PHISQ)
c     PHIDL(particle, r or i, state, xyz, +-)  values of shifted phis
cnodtc        with shifted bosepart multiplied in.
c     PHIDL_ALL(particle, r or i, state, xyz, +-)  values of shifted phis
c             particle index is for all the nucleons
c             particle index is for the operator nucleons
c    DETDL(particle, r or i, xyz, +-) -  shifted determs*bose
c
      complex*16 mats
      common /funci/
     &  mats(npardm, matdim),
     &  xyzs(npardm, 3), fs(npardm, 2),
     1  grdpsj(npardm, 2, 4), grdbos(npardm, 4), grdsin(npardm, 2, 4),
     &  bosedl(lnpar, 3, 2), phis(npardm, 2, numstate),
     4  phidl(lnpar, 2, numstate, 3, 2),
     &  phidl_all(npardm, 2, numstate, 3, 2),
     2  phinrm(npardm, 2), detdl(lnpar, 2, 3, 2),
     &  mcubes(npardm)
c
c
c     array of functions of two particles - gs(ij, alpha)
c     alpha  contents
c       1    rij**2
c       2    delta(x) for interp
c       3    u(i,j)
c       4    real( det(particle i replaced with j's location) / normal )  100   
c                note: i < j.  if i,j are in the same det,              cdet
c              the value 1 is left unchanged.                           cdet
c       5    real( det(particle j replaced with i's location) / normal )cdet
c                note: i < j.  if i,j are in the same det,              cdet
c              the value 1 is left unchanged.                           cdet
c       6    Imag part corresponding to 4
c       7    Imag part corresponding to 5
c       8    real( 1/psij (lap(i)+lap(j)) psij )
c       9    imag( 1/psij (lap(i)+lap(j)) psij )
c      10    1/r(ij)**2  (for scaling r**2 out of cubes)                  110   
c      11    not used
c      12    weight(ij) = weight factor for ij flip.  maintained and
c            used by movjas
c
c     ij = (1/2)*(j-1)*(j-2) + i    for i < j .
c
c     items 1, 3, 4, 5, & 12 are maintained by movjas
c     items 2, 8, 9 & ncubes, xyzij are computed by kinet1
c
c     ncubes(ij) - spline index for functions of rij - setup by kinet1    120   
c
c     xyzij(ij,1-2-3) - r(i)-r(j) - this is setup by kinet1
c
c     see wirwrk below for an array equivalenced to funcij
c
      parameter  ( ngsdim = nmpair_all+1, numgs = 12 )                  /funcij/
c
      common /funcij/ gs(ngsdim, numgs), xyzij(ngsdim, 3),
     1   ncubes(ngsdim), ijall(npardm, numpar_all)
c
crhok>/rhokbl/
c
      complex*16 dets                                                   /determ/
      common /determ/  dets(lendets)
c                                                                       cdet
c     mat(j',i) is the matrix  phi(i)(r(j))  where                      cdet
c          j = detpar(j'',k) and k is the determinant                   cdet
c          j'' = j' - offset for determ k                               cdet
c          1 =< i =< npardet(k)                                         cdet
c          1 =< j'' =< npardet(k)                                       cdet
c          1 =< j =< numpar_all                                         cdet
c     matinv(i,j')  is the inverse of  mat(j',i) ; i.e.                 cdet
c         the i still refers to phi(i) so we have made a transpose      cdet
c                                                                         140   
      complex*16  matins                                                /matbl/
      common /matibl/  matins(matdim, numpar_all)
c
      parameter ( numubs = 8, numus = 8 , numvs = 14 , numv3s = 3 )     numbspar
      parameter ( numv14 = 15 , numf6 = 6 , numf8 = 8 , numu3s = 3 )
c
      equivalence ( firstparams, rijtny )                               /params/
      integer derpow, fijkt2
      common /params/  rijtny, ribig, rcp,
     &  ua, uc, u0, b3pi, b30, ampi,
     &  wavalph(4), wavbeta(4),
     &  seedin, rmin, rmax, step,
     &  uwt(numus), vwt(numvs), tywt(2), u3wt(2),
     &  scale1, scale2, smlnum, rhosca,
     &  rh2scl, rstmin, rdistr,
     &  prmrad(30),
     &  fijkt1, fijkt3,
     &  u3eps, u3delt, u3b3pi, u3b30, u3scale, u3ampi,
     &   u30, u3a, u3c,
     &  ipotyp, ipot3, itypij, itypijdw, itypwav, npts1, npts2,
     &  nrho1, nrho12, ityprd, nptsu0, ifijkt, iu3typ, iu3loc,
     &  fijkt2, derpow, akmax, qalpha, qbeta, numks, nqtype,
     &  qc1, qc2
c
      common /params/  fijprm(8), wstep1, wstep2,
     &        fijprmdw(8), wstep1dw, wstep2dw
cf6lr      common /params/  f6esep(4), f6eta(2), f6scal(4), f6ac(6),
cf6lr     1  f6aa(6), f6ar(6), f6alph(6), f6beta(6), f6gamm(6), wstep1
c
      integer wtpow1
      common /params/  wtpow2, wtlim, wtfac2, wtfac3, wtfac4,
     1  wtafac, wtbfac, wtcfac, wthp1, wthp2, wtpow1, iprmsp2
cnodt      common /params/   wtpow1, wtfac2, wtpow2,
cnodt     1  wtfac3, wtpow3, wtfac4, wtpow4, wtafac, wtbfac, wtcfac,
cnodt     2  wthp1, wthp2, rcp
cray      parameter (numwts = 11)
cnodtcray      parameter (numwts = 12)
      parameter (numwts = 11)
      dimension wtfacs(numwts)
      equivalence ( wtfacs(1), wtpow2 )
c
      common /params/ lastparams
c
      equivalence ( firstoption, isavin )                               /option/
      logical dov3, doke, dorho, dorh12, dorhop, dorhok, dofc,
     &  u56u14, thresw, foursw, discsw,
     &  elimsw, f8sw, dov2, doquas, doeep, dbugsw, tracsw,
     &  douijk, dofdiag, do4clwr
      common /option/ isavin, dov3, doke, dorho, dorh12, dorhop,
     1  ispcil, dorhok, dofc, u56u14, n56, thresw, foursw, discsw,
     &  elimsw, f8sw, dov2, doquas, doeep, ndebug, dbugsw, tracsw,
     &  douijk, dofdiag, do4clwr
      common /option/ lastoption
c
      integer allnodes, anynode                                         /paralle
      parameter ( allnodes = -1 ,  anynode = -1 ,  masternode = 0 )
      parameter ( lenint1 = 1 ,  lenint = 4 ,  lenreal = 8 ,
     &  lenreal4 = 4 )
cray      parameter ( lenint1 = 8 ,  lenint = 8 ,  lenreal = 8 ,
cray     &  lenreal4 = 8 )
c
      parameter ( kp4params = 10,  kp4option = 11,  kp4dl0 = 12,
     &   kp4stepbltoslv = 13, kp4stepblfrmslv = 14,
     &   kp4values = 15, kp4times = 16 )
c
      logical master, slave
      common / parallel /  master, slave, myid, numnodes
c
      include 'mpif.h'
c                                                                         150   
cMPI
      integer argc, argv, status(3), st_count, st_tag, st_source
      equivalence ( status(1), st_count ), ( status(2), st_source ),
     &   ( status(3), st_tag )
cMPI
      integer p4dbg
      parameter ( p4dbg = 0 )
c
      parameter ( maxslave = 256 )
      dimension numslavees(maxslave)                                      160   
      logical donesent(maxslave)
c
c     the block that we send to and from the slaves
c
      parameter ( lenstepstrt = 2*nmpair*lenint1 + 8*lenint
     &     + (10+2*lendets+3*npardm)*lenreal ,
     &   lenstepstrt_rl = (lenstepstrt+lenreal-1)/lenreal ,
     &   maxbuf4 = 30000 ,
     &   lenstepblock = lenstepstrt + lenreal4*maxbuf4 )
      integer*1 ijordr_bl                                                 170   
      complex*16  dets_bl
      real*4 buf4
      dimension  stepbl(lenstepstrt_rl), stepbl2(lenstepstrt_rl)
      common /stepblock/ idslave_bl, job_bl, icode, ie_bl, nbd1, nbd2,
     &   nok, jnk_bl, dl_bl, psinsq_bl, wavprod_bl, f3c_bl, weight_bl,
     &   phibol, dseed_bl, seedfl_bl, oldphiwt_bl, oldpsijsq_bl,
     &   dets_bl(lendets), xyzs_bl(npardm, 3), ijordr_bl(nmpair, 2),
     &   buf4(maxbuf4)
      equivalence ( stepbl(1), idslave_bl ), ( stepbl2(1), idslave_bl2 )
c                                                                         180   
c     arrays used to buffer stuff for difference calculations           cp4
c                                                                       cp4
      parameter ( maxdifes = 35000 ,  maxnewes = 500 ,                  cp4
     &   maxoldvals = 200 ,  maxntrace = 100 , maxdifstepblk = 7000 )   cp4
      integer*1  difproc(maxdifes)                                      cp4
      integer*2  difold(maxdifes), difnew(maxdifes), oldie(maxoldvals), cp4
     &   newie(maxnewes)                                                cp4
      real*8  oldblocks(maxntrace, maxoldvals),                         cp4
     &   newblocks(maxdifstepblk, maxnewes)                             cp4
c                                                                         190   
c     hb2onm = hbar**2/m(nucleon)  in  mev fm**2
c
      common /cnstnt/ pi, hb2onm                                        /cnstnt/
c
      common /ranblk/  dseed, dranfc                                    /ranblk/
c
c     following common contains all the things we form averages
c     of.
c
      parameter ( nvops=7,  nvf8op = 15,                                numopspa
     1  nwrpul = 11, nwprt = 5, nwops = nwprt+nwrpul, nvwops = 14,
     2  nbugop = 20 )
      common /values/                                                   /values/
     1  phisq, phiwt, u2, phib, psijsq, wvprd, wvprdsq, f3cval, f3csq,
     &  t1wav(4), t1trms(3), t1cm, t1sum, et1ana, t1s(nm1cl+1),
     2  d2s(nm2cl+1), v2s(nm2cl+1,nvops+1), t2s(nm2cl+1),
     4  d2f8s(nm2cl+1), v2f8s(nm2cl+1,nvf8op+1), t2f8s(nm2cl+1),
     6  d3s(nm3cl+1), v3s(nm3cl+1,nvops+1),
     7  w3s(nm3cl+1,nwops), t3s(nm3cl+1),
     8  d4s(nm4cl+1), d4alls(nm4cl+1), v4s(nm4cl+1,nvops+1),
     9  w4s(nm4cl+1,nwops), t4s(nm4cl+1),
     b  d22s(nm22cl+1), t12s(nm12cl+1), t13s(nm13cl+1),
     &  v22s(nm22cl+1,nvops+1), t22s(nm22cl+1),
     &  rsq1s(nm1cl+1,nmrh1), rsq2s(nm2cl+1,nmrh1),
     &  rsq3s(nm3cl+1,nmrh1), rsq4s(nm4cl+1,nmrh1),
     a  bugops(nbugop)
cfd>/fdiagcm/
c
      common /values/ lastvalues
      dimension vals(1)
      equivalence ( vals(1), phisq )
c
      common /oldval/ oldphiwt, oldpsijsq, valold(1)
c
      real*4 delta, etime_                                                210   
      type tb_type                                                      cnoncray
         sequence                                                       cnoncray
         real*4 usrtime                                                 cnoncray
         real*4 systime                                                 cnoncray
      end type                                                          cnoncray
      type (tb_type) etime_struct                                       cnoncray
c
      parameter ( nummaintim = 15, num3ketim = 6, num4ketim = 6+5,      /timblk/
     &   num4u3rtim = 4 ,
     &    numothertim = num3ketim+num4ketim+num4u3rtim,
     &     numtim = nummaintim + numothertim )
c
      common /timblk/ tmov, tkin1, te2, te2f8, td3, tv3, tk3,
     1  td4, tv4, tk4, tdis,
     2  tother, tottme, tstart, tottim,
     &  tim3ke(num3ketim), tim4ke(num4ketim), tim4u3r(num4u3rtim),
     3  flpmov, flpkn1, flpe2, flp2f8, flpd3, flpv3, flpk3,
     4  flpd4, flpv4, flpk4, flpdis,
     5  flpoth, totflp, flpnone(2),
     &  flp3ke(num3ketim), flp4ke(num4ketim), flp4u3r(num4u3rtim)
      parameter ( nump4tim = 9 )
      common /timblk/  tmyid, tslavwaits, tslavwaitr, tmclock, tdsum,     220   
     &  tdusr, tdsys, tmastwait, tmastfull
      parameter ( lentimes = (2*numtim+nump4tim)*lenreal )              cp4
      dimension times(numtim), flop(numtim), flops(numtim),
     1  timper(numtim), times2(2*numtim+nump4tim)
      equivalence (times(1), tmov),  (flop(1), flpmov)
      character*20 othertimname(numothertim)
      data othertimname /
     & '3b KE: inline', '       kinset',
     & '       mkjas3', '       thru3r', '       dopp&dopn',
     & '       thrsum',                                                   230   
     & '4b KE: inline',      '       kinset',
     & '       mkjas4, u3r', '       fouru3', '       dopp&dopn',
     & '       fousum',
     & '      t=2  pp','      t=1  pp','      t=1  pn',
     & '      t=0  pp','      t=0  pn',
     & '4b u3r: start', '        doapp', '        sums',
     &   '        final' /
c
      parameter ( npairneut = (numneut*(numneut-1))/2 ,                 twoparam
     1  nm2t1 = npairprot+npairneut , nm2t0 = nmpair-nm2t1 ,
     2  ln2t1 = nm2t1+1, ln2t0 = nm2t0+1 )
      parameter ( n2t1s0 = nmpu*nmpd+nmnu*nmnd ,
     &  nmpuprs = nmpu*(nmpu-1)/2 ,  nmpdprs = nmpd*(nmpd-1)/2 ,
     &  nmnuprs = nmnu*(nmnu-1)/2 ,  nmndprs = nmnd*(nmnd-1)/2 ,
     &  n2dbl1 = 4*( nmpuprs*(nmpd*numneut+nmnu*nmnd)
     &             + nmpdprs*(nmpu*numneut+nmnu*nmnd)
     &             + nmnuprs*(nmnd*numprot+nmpu*nmpd)
     &             + nmndprs*(nmnu*numprot+nmpu*nmpd) ) ,
     &  n2dbl2 = 4*( nmpuprs*(nmpdprs+nmnuprs+nmndprs)
     &    + nmpdprs*(nmnuprs+nmndprs) + nmnuprs*nmndprs ) )
      integer*1 ijordr, ijordr_walk                                     /twocom/
      common /twocom/  d2ij(lnpair, 2),
     &    rijkl(lnpair, nmpair),
     &  ijgs(lnpair), ijsrt2(lnpair),
     1  ijgs2(lnpair), kpnt2(numpar-1, numpar), ijd2(lnpair),
     2  ijs2(lnpair, 2), ijops(lnpar, numpar), ijsops(lnpair, 2),
     4  kpp(ln2t1, 4), kpn(ln2t0, 8), ijwlkmap(nmpair)
c
      integer one2t1, one2t0
      common /twocom/  i2dbl1(n2dbl1+1, 5),
     3  i2dbl2(n2dbl2+1, 5), one2t1(ln2t1),  one2t0(ln2t0),
     3  ippud(n2t1s0, 2), ipnab(ln2t0)
c
ceep      integer one2t1f, one2t0f
ceep      common /twocom/  one2t1f(ln2t1, 2), one2t0f(ln2t0, 2)
c
      common /twocom/  ijordr(nmpair, 2), ijordr_walk(nmpair_walk, 2)
      parameter ( nmvrh1 = maxrho*(nm1cl+nm2cl+nm3cl+nm4cl)*nmrh1,      /densty/
     &    nmvrh2 = maxrho*(nm2cl+nm3cl+nm4cl)*nmrh2,
     &    nmvrh = nmvrh1+nmvrh2 ,
     &  nmtrh1 = 5*maxrho*nmrh1 ,  nmtrh2 = 4*maxrho*nmrh2,
     &    nmtrh = nmtrh1+nmtrh2 )
c
      common /densty/  nmdorh, nmdor2, nmdovrho, njnkrh,
     &  rhodel, rhoinv, rh2del, rh2dln,
     1  rh1v1(maxrho, nm1cl, nmrh1), rh1v2(maxrho, nm2cl, nmrh1),
     1   rh1v3(maxrho, nm3cl, nmrh1), rh1v4(maxrho, nm4cl, nmrh1),
     1  rh2v2(maxrho, nm2cl, nmrh2), rh2v3(maxrho, nm3cl, nmrh2),
     1   rh2v4(maxrho, nm4cl, nmrh2),
     2  ib1(numpar), ib12(nmpair)
      dimension rhv(nmvrh)
      equivalence ( rh1v1(1,1,1), rhv(1) )
cquas>/quasbl/
cquas      dimension quasblk(1)
cquas      equivalence ( quasblk(1), htrm )
c
cpi>/pionbl/
c
      complex*16 olddet(lendets)
      character*8  thedat, thetim, tdat2, ttim2, tdat3, ttim3
      character*40 cpuid                                                  250   
      character*32 hostname
      character*80 title, title2
      character*50 walkfl, savefl
c
      common /title/ title
c
      integer verson , ver2, savver, savvr2
c
c     version history
c                                                                         260   
c     3 - 12/2?/88 - permutation order written on tape
c     4 -  1/2?/89 - t1byop added to /values/ (for subset of clusters)
c     5 -  2/14/89 - three-body correlation parameters added
c     6 -  3/22/89 - l.s spots added in values  (actually done around
c                    3/15 and  hx.w1 & hx.w2 made)
c     7 - 10/29/89 - fijk parameters added
c     8 - 12/14/89 - ifijkt & iu3typ, iu3loc
c     9 - 2/7/90 - 14 wops instead of 3
c    10 - 5/11/90 - bars all removed
c    11 - 9/6/90 - rho(k) parameters                                      270   
c    12 - 9/12/90 - wtfac2 added; wtfac5 removed
c    13 - 10/22/90 - determinants and mixed rho removed
c    15 - 3/11/91 - 4 sets of permutations for u(5&6) after u(1-4)
c    16 - 9/10/91 - backflow *****DEFUNCT*****
c         11/21/91 - go back to version 15
c    17 - 2/19/92 - generalized cluster expansion.
c    18 - 3/7/92 - r**2 added to values; all new file system
c    19 - 5/14/92 - qalpha, qbeta for rho(k)
c    20 - 6/20/92 - complex determs
c    21 - 8/9/92 - more stuff in DELTRC array; VERTITLE. WTLIM            280   
c    22 - 6/11/93 - surface wave parameters
c    23 - 6/18/93 - more surface wave parameters
c    24 - 8/10/93 - remove i56 from ijordr; ijordr in output file
c    25 - 8/27/93 - add f3c(i,j,k) params
c    26 - 9/22/93 -  new Vijk, Uijk params
c
c     savver is the version of the saveun info:
c     1 - 3/12/90 - first version - rho(r12) stuff
c     2 - 8/31/90 - rho(k) blocks
c     3 - 10/3/90 - fc(k) blocks                                          290   
c     4 - 10/22/90 - determinants and mixed rho removed
c     5 - 2/19/92 - generalized cluster expansion; all new file system.
c
      data verson /26/, savver /5/
c
      character*24 header, headuc, head2
      data header /'nuclei random walk      '/,
     &     headuc /'NUCLEI RANDOM WALK      '/,
     1   head2/'*** Nothing was read ***'/
      character*6 firlas(2)                                               300   
      data firlas / 'first', 'last' /
c
      integer derpw2, mdebug(30)
      logical  tripsw, redosw, savesw, walksw, contsw,
     &  newwsw, slavedone, walkdone, oldfull, newfull, frstsw, do4clrd
      data frstsw /.true./
c
      character*50 vertitle                                             vertitle
      data vertitle / '16O:  MPI demonstration version' /
      character*50 vertitle2
c                                                                         310   
      dimension xyz_walk(numpar_walk+1,3)
c
      integer time_                                                     cnoncray
cray      complex*16 zdum
cray      real*8 imag
cray      imag(zdum) = aimag(zdum)
c
c
c *******************************************************
      logsw = .true.                                                      320   
      logun = 0
      backsw = .false.
c *******************************************************
c
      job = 0
 1    tfirst = second(dummy)
      job = job+1
      wallstart = time_()                                               cnoncray
      call date (thedat)                                                  330   
      call clock ( thetim )
      call cmputr ( cpuid, hostname )
      tdat3 = thedat
      ttim3 = thetim
      do   i = 1, numtim
         times(i) = 0
         flop(i) = 0
      enddo
c
      if (frstsw) then                                                    340   
         numnodes = 1
         master = .true.
         slave = .false.
         tjobcpu = 0
         tjobflp = 0
         tjobwallst = wallstart
         jobdone = 0
c                                                                       cp4
c     setup for MPI parallel processing                                 cp4
c                                                                         350   
         call MPI_Init_( ierr )
         call MPI_Comm_rank_( MPI_COMM_WORLD, myid, ierr )
         call MPI_Comm_size_( MPI_COMM_WORLD, numnodes, ierr )
ccMPIcp4         call p4crpg
ccMPIcp4         myid = p4myid()
ccMPIcp4         numnodes = p4ntotids()
         master = myid .eq. masternode                                  cp4
         slave = .not. master                                           cp4
         wallst = time_() - wallstart                                   cp4
         write (p4dbg, 2) thetim, hostname, myid, numnodes, wallst        360   
  2      format (1x, a8, 1x, a24, ' is node', i4, ' of', i4,            cp4
     1      ' nodes; start time:', f6.0 )                               cp4
         if ( numnodes .le. 1 )  call stop ('Too few nodes', numnodes)  cp4
         frstsw = master                                                cp4
      endif
c
      do  i = 1, maxslave                                               cp4
         numslavees(i) = 0                                              cp4
         donesent(i) = .false.                                          cp4
      enddo                                                               370   
      tfir = second(dummy)                                              cp4
      wallst = time_()                                                  cnoncray
      write (p4dbg,*) ' Node', myid, ' init time (cpu, elapsed):',      cdbcp4
     &  tfir-tfirst, wallst-wallstart                                   cdbcp4
      tstrtp4 = wallst - wallstart                                      cp4
      tfirst = tfir                                                     cp4
      wallstart = wallst                                                cp4
      tmastwait = 0                                                     cp4
      tslavwaits = 0                                                    cp4
      tslavwaitr = 0                                                      380   
      delta = etime_(etime_struct)                                      cnoncray
      delta1 = delta                                                    cnoncray
      usr1 = usrtime                                                    cnoncray
      sys1 = systime                                                    cnoncray
      amclock1 = .01*mclock()                                           cnoncray
      write (p4dbg,*) ' mclock, dtime at start:', myid,                 cdbcp4
     &   amclock1, delta, usrtime, systime                              cdbcp4
c
c     use ddt to set  iabort > 0  to stop
c                                                                         390   
      iabort = 0
      tmovst = 0
c
c     cheap!
c
      foursw = .false.
      foursw = .true.                                                   cc4
      thresw = .false.
      thresw = .true.                                                   cc3
      discsw = .false.                                                    400   
cdisc      discsw = .true.
      f8sw = .false.
      f8sw = .true.                                                     cf8f6
      doquas = .false.
cquas      doquas = .true.
      doeep = .false.
ceep      doeep = .true.
c
      if ( discsw .and. mxdis2 .eq. 1 ) then                            cdet
         call stop( 'Must have mxdis2=2 for disconnected!', mxdis2)       410   
      endif                                                             cdet
c
      elimsw = numpar_all .ne. numpar_walk
c
c     dseed = 0  means use the default
c
c     isavin controls lots of stuff.  it is a 9 decimal digit
c     number:  isavin = opqrsthdu.  the digits are split apart into
c     variables and have the following meanings:
c     digit  variable  value  function                                    420   
c       u     iwlksv   controls i/o units walkun & saveun
c                        0  files are not used
c                        1  random walk written to walkun & restart
c                               to saveun
c                        2  random walk continued
c                        3  walk on walkun reused with new correlation.
c                           If provided, difference restart
c                           written on saveun.
c                        4  difference calc continued
c                        5  restart record just printed                   430   
c                        7  restart record only writen on saveun
c                        8  restart record only continued
c                 tracsw is .true. if iwlksv = 3 or 4 = doing dif calc.
c                 walksw is .true. for 1 or 2 = walk written.  the
c                           beginning of walkun is also written for 7.
c                 contsw is .true. for 2 or 8 = continuing old walk or d
c                 savesw is .t. for 1, 2, 4, 7, 8 and 3 if provided =
c                               saveun is written on
c                 redosw is .t. for 5
c                 newwsw is .t. for 1, 2, 7 = a new walk is started       440   
c       d     idorho  - controls calc. & printout of densities
c                        0    no densities done
c             dorho      1    rho(r) computed
c             dorh12     2     + rho(r12) & rho(pp)
c             dorhop     3     + rho(r12) op(p)
c       h     dov3       0    compute <Wijk> & <vij>
c             dov2       1    compute only <vij>
c                        2    do not compute potentials
c       t     doke       0    compute kinetic energy
c                        1    do not compute k.e.                         450   
c       s                0    check called for numpar_all =< 30
c                        1    check always called
c                    >=  2    check never called
c                        3    4-body diagnostics; no check
c                        4    write all 4-body clusters
c                        5    read 4-body clusters and sum - NO SLAVES!
c       r    tripsw      0    no triplets statistics
c                        1    compute triplets statistics
c       q    ispcil      0    normal calculation
c                        1    special calculation of density              460   
c                               fluctuations
c                        2    rho(k)
c                        3    fc(k)
c       p     idebug     0    no debug write ( 6, out
c                        1    line of energies for each eval
c               dbugsw set to .true. if t=1
c                        2    selected values written to unit 9; see
c                             last read from unit 5.
c       o     idbgsv     0    no debug output on unit 10
c                        1    accumu write debug stuff on 10              470   
c
c      some "types"
c
c     ityprd - see phifer
c     itypij - not used for f8lr
c     ifijkt - 0 = none
c     iu3typ - 0 = none
c     iu3loc - 1 = first; 2 = last
c
      ie = 0                                                              480   
      icyc = 0
      numks = 0
      nqtype = 0
      qalpha = 0
      do i = 1, lendets
         dets(i) = 1
         olddet(i) = 1
      enddo
c
c     The input is read only on the master node                           490   
c
      if ( slave ) go to 50
c
      call clock ( ttim2 )
      if ( logsw ) write (logun, 3) ttim2, vertitle
  3   format ( 1x, a8, ' Nucleus:  ', a50)
c
      write (6, 7) cpuid, thedat, thetim, vertitle
  7   format ( '1', 50x, 'N U C L E I '   /
     1  1x, 'Computer: ', a40, t100, a8, 2x, a8 /                         500   
     2  '0Nucleus:  ', a50 / )
      if (master) then
         write (6,8) numnodes                                           cp4
  8      format ( '0There are a total of', i4, ' nodes.' )              cp4
         write (6,*) 'The master is ', hostname
      endif
c
      npxxxx = -99999
      read (5, 13, end=990)  title
 13   format ( a80 )                                                      510   
      frstsw = .false.
      write (6, 14) title
 14   format ('0', a80)
      read ( 5, *, end=995 )
     1   npxxxx, ninitl, nmoves, nenerg, ngroup, rmin, rmax, step,
     2   seedin, derpow, logval, isavin
      if ( npxxxx .eq. -99999 )  go to 995
      if ( npxxxx .ne. numpar_all ) go to 955
c
      iwlksv = mod(isavin, 10)                                            520   
      walksw = iwlksv .eq. 1  .or.  iwlksv .eq. 2
      savesw = walksw  .or.  iwlksv .eq. 4  .or.  iwlksv .ge. 7
      contsw = iwlksv .eq. 2  .or.  iwlksv .eq. 4  .or.
     &   iwlksv .eq. 8
      tracsw = iwlksv .eq. 3  .or. iwlksv .eq. 4
      redosw = iwlksv .eq. 5
      newwsw = iwlksv .le. 1  .or.  iwlksv .eq. 7
c
      idorho = mod( isavin/10, 10 )
      dorho  = idorho .ge. 1                                              530   
      dorh12 = idorho .ge. 2
      dorhop = idorho .ge. 3
      dov3   = mod( isavin/100, 10 ) .eq. 0
      dov2   = mod( isavin/100, 10 ) .le. 1
      doke = mod( isavin/1000, 10 ) .ne. 1
      dofdiag = mod(isavin/10000, 10) .eq. 3
      do4clwr = mod(isavin/10000, 10) .eq. 4
      do4clrd = mod(isavin/10000, 10) .eq. 5
      dofdiag = dofdiag .or. do4clrd
      if ( dofdiag .or. do4clwr .or. do4clrd ) then                       540   
         i = 0
cfd         i = 1
         if ( i .eq. 0 ) call stop (
     &      'NOT compiled with FOUDIAF (CFD)', isavin )
         if ( do4clwr .or. do4clrd )  then
             write (6, *) 'Opening /tmp/spieper/4bdyclus'
             if (do4clwr)  open (unit=11, file='/tmp/spieper/4bdyclus',
     &         status='unknown', form='unformatted' )
             if (do4clrd)  open (unit=11, file='/tmp/spieper/4bdyclus',
     &         status='old', form='unformatted' )                         550   
         endif
      endif
      tripsw = mod( isavin/100 000, 10 ) .eq. 1
      ispcil = mod( isavin/1 000 000, 10 )
      dorhok = ispcil .eq. 2
      dofc = ispcil .eq. 3
      idebug = mod(isavin/10 000 000, 10)
      dbugsw = idebug .eq. 1
      idbgsv = mod(isavin/100 000 000, 10)
      ndebug = 0                                                          560   
c
      if ( seedin .eq. 0 .and. newwsw )  seedin = 759 6651
c
      if (iwlksv .ne. 0 )  then
         read (5, *, end=975) walkfl, savefl
         if (walkfl(1:1) .eq. ' ')  go to 940
         write (6, 17) '0', 'WALK', walkfl
 17      format ( a1, 'The ', a4, ' file is: ', a50 )
         if (savefl(1:1) .ne. ' ') then
            write (6, 17) ' ', 'SAVE', savefl                             570   
            savesw = savesw .or. tracsw
         else
            if ( savesw )  go to 940
         endif
         if (tracsw .and. nenerg .gt. maxdifes ) call stop (            cp4
     &     'Too many energies for buffers; max buffer',                 cp4
     &     maxdifes )                                                   cp4
      endif
c
      if ( newwsw )  go to 25                                             580   
c
c     get the input that was used to make the random walk tape
c
      open ( unit=walkun, file=walkfl, err=945, iostat=iostat,
     &  status='old', form='unformatted' )
c
      read (walkun, err=960, end=960) head2, tdat2, ttim2, ver2,
     &  title2, vertitle2
      if ( tracsw ) then
         write (6, 23) 'Following', head2, tdat2, ttim2, ver2,            590   
     &     verson, title2, vertitle2
      else
         write (6, 23) 'Continuing', head2, tdat2, ttim2, ver2,
     &     verson, title2, vertitle2
      endif
 23   format ( '0', a, a19, ' made on ', a8, ' at ', a8,
     2    5x, 'walk version =', i3, ',  current version =', i3 /
     3  1x, a80 / ' Nucleus:  ', a50 )
      if ( head2 .ne. header .and. head2 .ne. headuc )  go to 960
      if ( ver2 .lt. 21 )                                                 600   
     &    call stop ( ' **** version is too old:', 21 )
c
      if ( elimsw )  then
         write (6,*) ' Walk was made for ', numpar_walk,
     &      ' nucleons, computation has only ', numpar_all
         if (logsw) write (logun,*) numpar_walk, ' nucleons in walk; ',
     &     numpar_all, ' now'
      endif
c
      iisvin = isavin                                                     610   
      read (walkun) i, j, k, j, ngrptp, rmin, rmax, step, ddseed,
     1  derpw2, iisvin
      if ( i .ne. numpar_walk .or. ngroup .ne. ngrptp )  go to 965
      read (walkun)  npts1, scale1, npts2, scale2, smlnum,
     1  nrho1, rhosca, nrho12, rh2scl, rstmin, rdistr
      read (walkun) ityprd, prmrad, nptsu0
c
      if ( ver2 .ge. 23 ) then
         read (walkun) itypwav, wavalph, wavbeta
      else if ( ver2 .eq. 22 )  then                                      620   
         read (walkun) itypwav, wavalph
      else
         itypwav = 0
      endif
c
      read (walkun) itypij, fijprm, wstep1, wstep2                      cf8nm
ceepcf8nm      read (walkun) itypijdw, fijprmdw, wstep1dw, wstep2dw
cf6lr      read (walkun) itypij, f6esep, f6eta, f6scal, f6ac, f6aa, f6ar
cf6lr     1  f6alph, f6beta, f6gamm, wstep1
      read (walkun) ifijkt, fijkt1, fijkt2, fijkt3                        630   
c
      if ( ver2 .ge. 25 ) then
         read (walkun) qc1, qc2
      else
         qc1 = 0
         qc2 = 0
      endif
c
      if ( ver2 .ge. 26 ) then
         read (walkun) ipotyp, rcp, ipot3, ua, uc, u0, b3pi, b30, ampi    640   
      else
         read (walkun) ipotyp, ipot3, u0, ua, ampi, b3pi
         b30 = b3pi
         uc = .25*ua
         read (walkun) rcp
      endif
      read (walkun) uwt, vwt, tywt, wtfacs
c
      if ( ver2 .ge. 26 ) then
         read (walkun) iu3typ, iu3loc, u3eps, u3delt,                     650   
     1       u3b3pi, u3b30, u3scale, u3ampi, u3wt
      else
         read (walkun) iu3typ, iu3loc,
     1        u3eps, u3delt, u3ampi, u3b3pi, u3wt
         u3b30 = u3b3pi
         u3scale = 1
      endif
c
      read (walkun) numav2, numvl2, ntrac2
      if ( ver2 .ge. 19 )  then                                           660   
         read (walkun)  numks, akmax, nqtype, qalpha, qbeta
ceep      read (walkun)  idstwav, nqtype, numkin,
ceep     &  qmin, qstep, pfmin, pfstep, thmin, thstep
      else
         read (walkun)  numks, akmax, nqtype
         if ( dorhok )  call stop ( 'Version too old for rhok', 19)
      endif
      read (walkun) dl0
c
c     open the save file if it will be used.  read the begining           670   
c     if required
c
 25   if ( contsw .or. redosw ) then
         if ( savefl(1:1) .eq. ' ' )  go to 940
         open ( unit=saveun, file=savefl, err=945, iostat=iostat,
     &      status='old', form='unformatted' )
         read (saveun, end=980, err=980)  savvr2, ie, icyc,
     &      tdat3, ttim3
         if ( .not. tracsw )  then
            if ( tdat3 .ne. tdat2  .or.  ttim3 .ne. ttim2 ) then          680   
               write (6,*) '*** save and walk files do not agree: ',
     &            tdat3, tdat2, ttim3, ttim2
               call stop ('save and walk files do not agree', 0)
            else
               write (6,*) 'Continuing walk that has ',
     &            ie, ' energies in ', icyc, ' groups.'
               if (logsw)  write (logun,*) 'Continuing walk that has ',
     &            ie, ' energies in ', icyc, ' groups.'
            endif
         else                                                             690   
            write (6,*) 'Continuing difference calculation made on ',
     &         tdat3, ' at ', ttim3, ' that already has ',
     &         ie, ' energies in ', icyc, ' groups.'
            if (logsw) write (logun,*)
     &         'Continuing difference run that alread has ', ie,
     &           ' energies'
         endif
         if ( .not. elimsw )  then
            read (saveun, end=980, err=980)  xdseed, seedfl, dl, dets,
     1         psinsq, wtjunk, weight, xyzs, ijordr                       700   
         else
            read (saveun, end=980, err=980)  xdseed, seedfl, dl, dets,
     1         psinsq, wtjunk, weight, xyzs, ijordr_walk
         endif
      elseif (savesw) then
         if ( savefl(1:1) .eq. ' ' )  go to 940
         open ( unit=saveun, file=savefl, err=945, iostat=iostat,
     &      status='unknown', form='unformatted' )
      endif
c                                                                         710   
c     if a walk is being continued, we do not read the input parameters
c
      if ( redosw .or. (contsw .and. .not. tracsw) )  go to 40
c
      read (5, *, end=975)  npts1, scale1, npts2, scale2, smlnum,
     1   nrho1, rhosca, nrho12, rh2scl, rstmin, rdistr
c
      do 26  i = 1, 6                                                   cf8nm
         fijprm(i+2) = 0                                                cf8nm
 26   continue                                                            720   
      do 27  i = 1, 30
         prmrad(i) = 0
 27   continue
cf6lr      ityprd = 0
cf6lr      nptsu0 = 100
      read (5,*, end=975) ityprd, (prmrad(i),i=1,10), nptsu0            cf8nm
      if (ityprd .eq. 40 .or. ityprd .eq. 46)                           cf8nm
     1    read(5,*, end=975) (prmrad(i), i=11,20)                       cf8nm
      if (ityprd .eq. 30) read(5,*, end=975) (prmrad(i), i=1,numrad)    cf8nm
c                                                                         730   
      read (5,*, end=975) itypwav, wavalph, wavbeta
c
      itypij = 1
      read (5, *, end=975)  itypij,                                     cf8nm
     1      (fijprm(i),i=1,6), wstep1, wstep2                           cf8nm
ceepcf8nm      read (5, *, end=975)  itypijdw,
ceepcf8nm     1      (fijprmdw(i),i=1,6), wstep1dw, wstep2dw
cf6lr      read (5, *, end=975) itypij, f6esep, f6eta, f6scal, f6ac, f6a
cf6lr     1  f6ar, f6alph, f6beta, f6gamm, wstep1
      read (5, *, end=975)  ifijkt, fijkt1, fijkt2, fijkt3                740   
      read (5, *, end=975)  qc1, qc2
      read (5, *, end=975)  iu3typ, iu3loc, u3eps, u3delt,
     1       u3b3pi, u3b30, u3scale, u3ampi
      read (5, *, end=975) ipotyp, rcp,
     &   ipot3, ua, uc, u0, b3pi, b30, ampi
      uwt(1) = 1
      read (5,*, end=975) (uwt(i), i=2,numus), u3wt, vwt, tywt
cnodt      if ( newwsw )  read(5, *, end=975) wtpow1, wtfac2, wtpow2,
cnodt     1   wtfac3, wtpow3, wtfac4, wtpow4, wtafac, wtbfac, wtcfac,
cnodt     2  wthp1, wthp2                                                 750   
      if ( newwsw )  read(5, *, end=975) wtpow1, wtpow2, wtlim,         cdet
     1  wtfac2, wtfac3, wtfac4, wtafac, wtbfac, wtcfac,                 cdet
     2  wthp1, wthp2                                                    cdet
      if (dorhok .or. dofc)  read (5, *, end=975)  numks, akmax, nqtype,
     1   qalpha, qbeta
cquas      read (5, *, end=975) numks, akmax, nqtype, qalpha, qbeta
ceep      read (5, *, end=975)  idstwav, nqtype
ceep      if ( idstwav .eq. 0 )  read (5, *, end=975)  numkin,
ceep     &  qmin, qstep, pfmin, pfstep, thmin, thstep
cpi      read (5, *, end=975) ak                                          760   
c
 40   if (idebug .eq. 2 )  read (5, *, end=975)  ndebug,
     1  ( mdebug(i), i = 1, ndebug )
c
c     All of the input has been read.
c
 50   continue
c                                                                       cp4
c     send out (or receive) /params/ & /option/                         cp4
c     this may also be a grand stop signal                                770   
c                                                                       cp4
      lenparams = loc(lastparams) - loc(firstparams)                    cp4
      lenoption = loc(lastoption) - loc(firstoption)                    cp4
cMPI
      lenparams = (lenparams+7)/8
      lenoption = (lenoption+7)/8
cMPI
      if ( master )  then                                               cp4
ccMPIp4         call p4brdcst( kp4params, firstparams, lenparams, irc )
           call MPI_BCAST_(firstparams, lenparams,  MPI_DOUBLE,           780   
     &        masternode, MPI_COMM_WORLD, irc )
         write (p4dbg,*) ' Sent /params/, len, irc: ', lenparams, irc   cdbcp4
ccMPIp4         call p4brdcst( kp4option, firstoption, lenoption, irc )
           call MPI_BCAST_(firstoption, lenoption,  MPI_DOUBLE,
     &        masternode, MPI_COMM_WORLD, irc )
         write (p4dbg,*) ' Sent /option/, len, irc: ', lenoption, irc   cdbcp4
      else                                                              cp4
ccc         open ( unit=6, file = '/tmp/spieper/ft06' )                 cp4
ccc         write (p4dbg,*) ' slave', myid, ' has opened /tmp/spieper/ftcdbcp4
ccMPIp4         call p4recv ( kp4params, masternode, firstparams, lenpar  790   
ccMPIp4     &      igot, irc )
           call MPI_BCAST_(firstparams, lenparams,  MPI_DOUBLE,
     &        masternode, MPI_COMM_WORLD, irc )
         write (p4dbg,*) ' Node ', myid, ' got /params/: ', lenparams,  cdbcp4
     &       irc                                                        cdbcp4
c                                                                       cp4
         if ( firstparams .eq. -1 ) then                                cp4
            write (p4dbg,*) ' slave:', myid, ' STOPPING after',         cp4
     &               jobdone, ' energies'                               cp4
ccMPIp4            call p4cleanup                                         800   
            call MPI_Finalize_( irc )
            stop                                                        cp4
         endif                                                          cp4
c                                                                       cp4
ccMPIp4         call p4recv ( kp4option, masternode, firstoption, lenopt
ccMPIp4     &      igot, irc )
           call MPI_BCAST_(firstoption, lenoption,  MPI_DOUBLE,
     &        masternode, MPI_COMM_WORLD, irc )
         write (p4dbg,*) ' Node ', myid, ' got /option/: ', lenoption,  cdbcp4
     &       irc                                                          810   
c                                                                       cp4
c     define some things needed only on the master                      cp4
c                                                                       cp4
      ngroup = 1                                                        cp4
      nenerg = 1                                                        cp4
      nmoves = 1                                                        cp4
      ninitl = 1                                                        cp4
c                                                                       cp4
      endif                                                             cp4
c                                                                         820   
c     some general setup - all nodes do this
c     constants for amd's ranf
c
      d2p31 = 2 147 483 648 .d0
      dranfc = 65 547
      dseed = seedin
c
      ncycle = (nenerg+ngroup-1)/ngroup
      nenerg = ncycle*ngroup
      derstp = .5e0**derpow                                               830   
c
      u30 = u3delt*u0
      u3a = u3eps*ua
      u3c = u3eps*uc
      dov3 = dov3 .and. ( u0 .ne. 0 .or. ua .ne. 0 .or. uc .ne. 0 )
      douijk = iu3typ .ne. 0 .and.
     &   ( u30 .ne. 0 .or. u3a .ne. 0  .or. u3c .ne. 0 )
c
      u56u14 = .false.
      n56 = 1                                                             840   
c
c     only the master node shows the output
c
      if ( slave )  go to 200                                           cp4
c
      write (6, 51) numpar_all, ninitl, nmoves, nenerg, ngroup, rmin,
     1   rmax, step, seedin, derpow, derstp, logval, isavin
  51  format ( '0', i4, ' nucleons,', i8, ' initial moves,',
     1    i8, ' attempted steps each energy,',
     2    i8, ' energy evaluations in groups of ', i4 /                   850   
     2  '0r(min) =', f9.4, 5x, 'r(max) =', f7.2, 5x,
     3     'random walk step size =', f9.4,
     4     5x, 'random number seed =', f13.0 /
     4  1x, 'step size for numeric derivatives = .5**(',
     5     i2, ') =', g15.5, 5x, 'logging value #', i3,
     6     5x, 'options =', i13 )
      write (6, 57) npts1, scale1, npts2, scale2, smlnum
  57  format (
     1  '0r(i) interpolation uses', i4, ' points with scale =', f9.2 /
     3  ' for r(ij),', i4, ' points with scale =', f9.5,                  860   
     2    5x, 'min ln(psi) =', g12.3 )
      write ( 6, 63) nrho1, rhosca, nrho12, rh2scl
 63   format (
     1  '0One-body density grid has', i4, ' points ',
     2    'on the grid  r**2 / (', f7.3, ' + r**2 )' /
     3  ' two-body density grid has', i4, ' points ',
     4    'on the grid  r**2 / (', f7.3, ' + r**2 )' )
      write ( 6, 67) rstmin, rdistr
 67   format (
     5  '0initial distribution:  minimum separation =', f9.5,             870   
     6     5x, 'distribution radius =', f7.2 )
c
      write (6, 73) ityprd, (prmrad(i), i=1,10), nptsu0
 73   format ( /'0Single-particle radial wavefunction type', i3,
     1    ' with the woods-saxon parameters:' /
     1  ' r =', f6.3, 5x, 'a =', f6.3, 5x,
     2    'separation energy =', f6.3,
     2     5x, 'rho =', f6.3, 5x, 'alpha =', f6.3 /
     2  ' rso =', f6.3, 5x, 'aso =', f6.3, 5x, 'vso =', f8.4, f12.4,
     2    5x, ' initial depth =', f6.1, i9, ' points' )                   880   
      if (ityprd .eq. 40 .or. ityprd .eq. 46 )
     1   write (6, 74) (prmrad(i), i=11,20)
 74   format ( '0parameters for the l=1 well:' /
     1  ' r =', f6.3, 5x, 'a =', f6.3, 5x,
     2    'separation energy =', f6.3,
     2     5x, 'rho =', f6.3, 5x, 'alpha =', f6.3 /
     2  ' rso =', f6.3, 5x, 'aso =', f6.3, 5x, 'vso =', f8.4, f12.4,
     2    5x, ' initial depth =', f6.1 )
      if ( ityprd .eq. 30 )  write (6,78) (prmrad(i),i=1,numrad)
 78   format ( '0k-s for the fermi-gas bessel funcs:' /                   890   
     1   (1x, 5f10.5) )
c
      if ( itypwav .ne. 0 )  then
         write (6, 83)  itypwav, wavalph, wavbeta
 83      format ( '0Surface wave type', i4, '; alphas =', 4f10.6 /
     &      ' R for cutoff =', f5.2, ';   other params =', 3f10.6 )
      else
         write (6, 84)
 84      format ( '0No surface waves' )
      endif                                                               900   
c
      write (6, 103) '0Using nuclear-matter short-ranged two-body',     cf8nm
     &  itypij, (fijprm(i),i=1,6), wstep1, wstep2                       cf8nm
 103   format ( a, ' correlation function type', i4, ' with:' /         cf8nm
     2  ' alpha =', f7.3, 5x, 'beta(s) =', f7.3, 5x, 'beta(t) =', f7.3, cf8nm
     2   5x, 'd =', f7.3, 5x, 'dt =', f7.3,                             cf8nm
     3    5x, 'kf =', f7.4 /                                            cf8nm
     5    5x, 'computation grid has step sizes:', 2f10.5 )              cf8nm
ceepcf8nm      write (6, 103)
ceepcf8nm     & '0For hole-others: nuclear-matter short-ranged two-body'  910   
ceepcf8nm     &  itypijdw, (fijprmdw(i),i=1,6), wstep1dw, wstep2dw
c
cf6lr      write (6, 103) itypij, f6esep, f6eta, f6scal, f6ac, f6aa, f6a
cf6lr     1  f6alph, f6beta, f6gamm, wstep1
cf6lr 103   format ( '0Using long-ranged two-body',
cf6lr     1    ' correlation function type', i4, '  with:' /
cf6lr     2  ' esep =', 4f8.3, 6x, 'eta   =', 2f7.3,
cf6lr     2      6x, 'scales =', 4f7.3 /
cf6lr     2  ' ac   =', 6f8.3, ' aa    =', 6f8.3 /
cf6lr     3  ' ar   =', 6f8.3, ' alpha =', 6f8.3 /                        920   
cf6lr     3  ' beta =', 6f8.3, ' gamma =', 6f8.2 /
cf6lr     5    5x, 'computation grid has step size:', f10.5 )
c
      if ( u56u14 )  then
         write (6, *)  'Pair operators 5&6 done after 1-4'
      else
         write (6, *)  'All 6 pair operators done together'
      endif
c
      write (6, 107)  ifijkt, fijkt1, fijkt2, fijkt3, qc1, qc2            930   
 107  format ( '0fijk type', i3,
     1    ',  t1, t2, t3 =', f8.4, i4, f8.5 /
     &  ' f3c(i,j,k):  qc1 =', f8.4, 5x, 'qc2 =', f8.4 )
c
      write (6, 113)  iu3typ, firlas(iu3loc),
     1    u3eps, u3delt, u3b3pi, u3b30, u3scale, u3ampi
  113 format ( '0Parameters for three-body correlation type',
     1    i3, ' which acts ', a5, ':',
     1    ' epsilon =', f9.6, 5x, 'delta =', f9.6 /
     2  ' b(2-pi) =', f5.2, 5x, 'b(0) =', f5.2, 5x, 'scale =', f5.2,      940   
     2   5x, 'm(pi) =', f6.3 )
c
      write (6, 127) ipotyp, rcp, ipot3, ua, uc, u0, b3pi, b30, ampi
 127  format ( '0Interparticle potential type', i5 /
     1  ' Coulomb potential has proton charge radius =', f7.4 /
     1  ' three-body potential type', i3, 5x,
     &     'ua, uc =', 2f10.6,  5x, 'u0 =', f9.6 /
     3   5x, 'b(2-pi) =', f5.2, 5x, 'b(0) =', f5.2,
     2     5x, 'm(pi) =', f6.3 )
c                                                                         950   
      write (6, 133)  (uwt(i), i = 2, numus), u3wt, vwt, tywt
 133  format ( '0multipliers for the pair correlations:' /
     1  13x, 'tau    sigma   sig-tau   tensor  ten-tau',
     2     '     b      b-tau' /
     4  7x, f9.2, 6f9.2 /
     5  '0multipliers for three-body correlation:',
     6    5x, 'tensor:', f5.2, 5x, 'yukawa:', f5.2 /
     5  '0multipliers for the two-body potentials:' /
     1  '  central    tau    sigma   sig-tau   tensor  ten-tau',
     2     '     b      b-tau      q      q-tau    q-sig',                960   
     3     '   q-sigtau    bb    bb-tau' /
     4  1x, f6.2, 13f9.2 /
     5  '0multipliers for three-body potential:',
     6    5x, 'tensor:', f5.2, 5x, 'yukawa:', f5.2 )
c
cnodt      write (6, 143)  wtpow1, wtfac2, wtpow2, wtfac3, wtpow3,
cnodt     1  wtfac4, wtpow4, wtafac, wtbfac, wtcfac, wthp1, wthp2
cnodt 143  format ( '0powers and factors for random walk weight:',
cnodt     1     '  sum power:', f5.2 /
cnodt     2  ' factors and powers for clusters:' /                        970   
cnodt     3  '   2:', f9.5, f5.2, 6x,
cnodt     3  '   3:', f9.5, f5.2, 6x,  '   4:', f9.5, f5.2 /
cnodt     5  '0the weight function (h(rij)):' /
cnodt     6  ' a =', f8.3, 5x, 'b =', f8.3, 5x, 'c =', f8.3,
cnodt     7    5x, 'power 1 =', f5.2, 5x, 'power 2 =', f5.2 )
      write (6, 143) wtpow1, wtpow2, wtlim, wtfac2, wtfac3,             cdet
     1  wtfac4, wtafac, wtbfac, wtcfac, wthp1, wthp2                    cdet
 143  format ( '0powers and factors for random walk weight:' /          cdet
     1  ' interchange power:', i3, 5x, 'sum power:', f5.2 ,             cdet
     1     5x, 'weight limit:', g14.3 /                                   980   
     2  ' factors:  2-body:', f8.3, 6x,                                 cdet
     3  '   3-body:', f8.3, 6x,  '4-body:', f8.3 /                      cdet
     5  '0the weight function (h(rij)):' /                              cdet
     6  ' a =', f8.3, 5x, 'b =', f8.3, 5x, 'c =', f8.3,                 cdet
     7    5x, 'power 1 =', f5.2, 5x, 'power 2 =', f5.2 )                cdet
c
c
      if ( ndebug .ne. 0 )  then
         write (6, 147) ndebug, dbugun, (mdebug(i), i=1,ndebug)
 147     format ( '0', i4, ' walk variables written on unit', i3 /        990   
     1      ( 1x, 9i4 ) )
         write (dbugun, 147) ndebug, dbugun, (mdebug(i), i=1,ndebug)
      endif
crhok      if (dorhok)  write (6, 153)  numks, akmax, nqtype, qalpha,
crhok     &     qbeta
crhok 153  format ( '0rho(k) computed at', i4, ' k-values up to', f7.2,
crhok     1  ' fm(-1);', i4, ' splines used on grid of length',
crhok     2  f7.2, 5x, 'expoential scale =', f6.2 )
cquas      write (6, 153) numks, akmax, nqtype, qalpha, qbeta
cquas 153  format ( '0Quasi-hole(k) computed at', i4, ' k-values up to', 1000   
cquas     1  f7.2, ' fm(-1)' /
cquas     &  ' Quasi-hole weight type', i4, 5x, 'alpha, beta =',
cquas     &    2g15.4 )
ceep      write (6, 153)  idstwav, nqtype
ceep 153  format (  ' (e,e''p) Proton distorted wave type', i3, 5x,
ceep     &    ' Quasi-hole weight type', i4 )
ceep      if ( idstwav .eq. 0 )  write (6, 154)  numkin,
ceep     &  qmin, qstep, pfmin, pfstep, thmin, thstep
ceep 154  format ( ' Plane wave (e,e''p) for', i4, ' cases: ',
ceep     &  ' q min, step =', 2f6.2, 5x, 'pf min, step =', 2f6.2,        1010   
ceep     &  5x, 'theta min, step =', 2f6.2 )
cfc      if (dofc)  write (6, 154)  numks, akmax
cfc 154  format ( '0fc(k) computed at', i4, ' k-values up to', f7.2,
cfc     1  ' fm(-1)' )
cpi      write (6, 153) ak
cpi 153  format ( '0', 79('*') /
cpi     1   ' *  calculation of residual nucleus after removing',
cpi     2      ' a pion with k =', f6.2, ' fm-1', t80, '*' /
cpi     3   ' ', 79('*') / )
c                                                                        1020   
 200  pi = 3.14159 2653 e0
c
ceep      if ( idstwav .ne. 0 )  call dwset ( numkin, logun )
c
      initfp = 0
      rijtny = 1.e+10
      ribig = 0
      rrmax = rmax
crhok      rrmax = rmax+.5*qalpha
c                                                                        1030   
      rrmax2 = 2*rmax+4*wstep2
crhok      rrmax2 = rrmax2 + qalpha
c
      call setint ( nptij, npts2, scale2, rrmax,
     3  rrmax2 )
c
cfc      if (dofc)  call setfc ( akmax, numks, nptij, rijgrd, vs, npts2
c
c     to give things time to settle down, we increase rmax
c     we make it rmax to double size of grid for rho(k)                  1040   
c     For infinite well boundry, we cannot increase rmax
c
      rmaxdl = 2
crhok      rmaxdl = qalpha+2
      if ( ityprd .eq. 17 )  rmaxdl = 0
      ek = prmrad(3)
      ep = prmrad(10)
      call setrad ( ityprd, prmrad, nptsu0, rmax, rmaxdl,
     1    npts1, scale1 )
      prmrad(3) = ek                                                     1050   
      prmrad(10) = ep
c
      call settwo ( contsw )
      call setthr
c
c     we need setfou for diconnected setcls calls also
      call setfou                                                       cc4
cdisc      if ( .not. foursw )  call setfou
c
      call setcls                                                        1060   
c
c     setup arrays for flipping spin and isospin                        cdet
c                                                                       cdet
      call setflp ( numpar_all )                                        cdet
c
      call setacc ( numav, numval, ntrace,
     1   contsw .or. redosw, saveun, ie )
c
      call setrho ( dorho .and. (contsw .or. redosw), saveun )
c                                                                        1070   
crhok      if (dorhok)  call setrhk ( akmax, qalpha, numks, nqtype,
crhok     &   qbeta, contsw .or. redosw, saveun )
cquas      call setqua ( contsw, saveun )
ceep      call seteep ( numkin, idstwav, nqtype, qmin, qstep,
ceep     &  pfmin, pfstep, thmin, thstep, contsw, saveun )
cfc      if (dofc)  call setfc2 (  contsw .or. redosw, saveun )
cfd      if (dofdiag)  then
cfd          call foudiag(1)
cfd          if  (contsw .or. redosw )
cfd     &       read ( saveun, end=980 ) fdcount, fdsums                 1080   
cfd      endif
c
c  <<<<<< the triplet counter is used only for diagnostics >>>>>>>>
c
ctrip      if (tripsw)  call settrp
cfluc      if ( ispcil .eq. 1 )  call setnfl
c
ctstphi      call tstphs
c
c     initial position and randomizing is done only on the master        1090   
c
      if ( slave )  go to 390
c
c     if we are continuing, position the walk file if it is being used
c
      if (contsw) then
c
         if ( walksw .or. tracsw )  then
            do 229  ietape = 1, ie
               read (walkun, end=950, err=950)  x                        1100   
               if ( mod(ietape,ngroup) .eq. 0 )
     &            read (walkun, end=970) x
 229        continue
         endif
c
         write (6, 233) ie, d2p31*xdseed
 233     format ('0restart record, seed =', i5, f25.5)
         dseed = xdseed
         olddl = dl
         call set2 ( dl0, dl, psinsq, wavprod, f3c, ispcil, weight )     1110   
         write (6, 253) dl0, olddl, dl
 253     format ( ' above initial psi shifted by ln(bias) =', g15.4,
     &     5x, 'old, new dl =', 2g20.10 )
c
      else if ( newwsw )  then
c
c     here we are starting a new walk
c
         dseed = dseed/d2p31
         if ( dseed .ge. .99999 )  then                                  1120   
            write (6,*) ' *** Seed is too large!', dseed, dseed*d2p31
            call stop ('Seed is too large!', 0)
         endif
         call set ( 0., dl, psinsq, wavprod, f3c, ispcil, weight )
      endif
c
      tlast = second(tlast)
      write (6, 297) tlast-tfirst
 297  format ( '0initialization time:', f10.3, ' seconds.' )
      call clock ( ttim2 )                                               1130   
      if (logsw)  write (logun, 298) ttim2, tlast
 298  format ( a8, ' Setup done, CPU time =', f8.3 )
c
c     make the initial moves
c
      tstart = tlast
c
      if ( .not. (contsw .or. tracsw)  )  then
c
         call movjas ( ninitl, numbd1, numbd2, numok,                    1140   
     2      dl, psinsq, wavprod, f3c, weight, numflp, ispcil )
         tlast = second(tlast)
         tmovst = tlast-tstart
         psib = 2.444e244
         if ( abs(dl) .lt. 2750 )  psib = exp(2*dl)
         write (6, 323) ninitl, numbd1, numbd2, numok, dl, psib
 323     format ( '0', i6, ' initial moves;    rejected:', 2i6,
     1      ';    accepted:', i6 /
     4      '0current ln(psi(bose)) =',  g14.5,
     5      5x, 'psi(bose)**2 =', g17.4  )                               1150   
         call clock ( ttim2 )
         if (logsw)  write (logun, 324) ttim2, tlast
 324     format ( a8, ' Initial moves done, total CPU time =', f8.3 )
c
         psi = psib*wavprod**2 * f3c**2
         if ( psi .lt. 1e200 )  psi = psi*psinsq
         write (6, 327) psinsq, wavprod**2, f3c**2, psi, weight
 327     format ( ' psi(boltzman)**2 =', g16.5,
     1      3x, '(surf wave)**2 =', g16.5, 3x, 'f3c**2 =', g16.5 /
     &      1x, 'psi(jastrow)**2 =', g16.5, 5x,                          1160   
     3      ' weight for random walk:', g17.5 )
         write (6, 328) dets                                            cdet
 328     format (' determinants =', (t17, 4g17.5) )                     cdet
         dl0 = dl + .5*log(abs(psinsq))
         write (6, 333) dl0
 333     format ( '0reseting psi(jastro) to current values;' /
     1      ' ln(bias) =', g15.5 )
      endif
c
c     start a random walk tape                                           1170   
c
      if ( (walksw .or. savesw) .and. .not. (contsw .or. tracsw) )  then
c
         open ( unit=walkun, file=walkfl, err=945, iostat=iostat,
     &      status='unknown', form='unformatted' )
         rewind walkun
c
         write (walkun) header, thedat, thetim, verson, title,
     &     vertitle
         write (walkun) numpar_all, ninitl, nmoves, nenerg, ngroup,      1180   
     1      rmin, rmax, step, ddseed, derpow, isavin
         write (walkun)  npts1, scale1, npts2, scale2, smlnum,
     1      nrho1, rhosca, nrho12, rh2scl, rstmin, rdistr
         write (walkun) ityprd, prmrad, nptsu0
         write (walkun) itypwav, wavalph, wavbeta
         write (walkun) itypij, fijprm, wstep1, wstep2                  cf8nm
ceepcf8nm         write (walkun) itypijdw, fijprmdw, wstep1dw, wstep2dw
cf6lr         write (walkun) itypij, f6esep, f6eta, f6scal, f6ac,
cf6lr     1      f6aa, f6ar, f6alph, f6beta, f6gamm, wstep1
         write (walkun) ifijkt, fijkt1, fijkt2, fijkt3                   1190   
         write (walkun) qc1, qc2
         write (walkun) ipotyp, rcp, ipot3, ua, uc, u0, b3pi, b30, ampi
         write (walkun) uwt, vwt, tywt, wtfacs
         write (walkun) iu3typ, iu3loc, u3eps, u3delt,
     1       u3b3pi, u3b30, u3scale, u3ampi, u3wt
         write (walkun) numav, numval, ntrace
         write (walkun)  numks, akmax, nqtype, qalpha, qbeta
ceep      write (walkun)  idstwav, nqtype, numkin,
ceep     &  qmin, qstep, pfmin, pfstep, thmin, thstep
         write (walkun)  dl0                                             1200   
         if ( .not. walksw )  close ( unit=walkun )
      endif
c
      if (dbugsw)  write (6, 373)
 373  format ( '0information for each energy is:' /
     1  ' ie, ln(bose), psinsq, psij, ',
     2    'et, ev2, e' )
c
c     All the processors need to know  DL0
c                                                                        1210   
 390  continue
      call clock ( ttim2 )                                              cp4
      if ( master ) then                                                cp4
ccMPIp4         call p4brdcst ( kp4dl0, dl0, lenreal, irc )
           call MPI_BCAST_( dl0, 1,  MPI_DOUBLE,
     &        masternode, MPI_COMM_WORLD, irc )
         write (p4dbg,*) ttim2, ' Sent DL0: ', dl0                      cp4
      else                                                              cp4
ccMPIp4         call p4recv ( kp4dl0, masternode, dl0, lenreal, igot, ir
           call MPI_BCAST_( dl0, 1,  MPI_DOUBLE,                         1220   
     &        masternode, MPI_COMM_WORLD, irc )
         write (p4dbg,*) ttim2, ' Node, job, dl0:', myid, job,          cp4
     &     dl0                                                          cp4
c                                                                       cp4
c     send back a signal indicating the slave is ready for a position   cp4
c                                                                       cp4
         job_bl = job                                                   cp4
         idslave_bl = myid                                              cp4
         icode = -2                                                     cp4
         t11 = time_()                                                   1230   
ccMPIp4         call p4send ( kp4stepblfrmslv, masternode, stepbl,
ccMPIp4     &      3*lenint, irc )
         call MPI_Send_( stepbl, 2, MPI_DOUBLE, masternode,
     &                 kp4stepblfrmslv,  MPI_COMM_WORLD, irc )
         tslavwaits = tslavwaits + time_()-t11                          cp4
      endif                                                             cp4
c
c     loop over the cycles; print partial sums at end of each group
c     we do not use do loops so the slaves can function correctly
c                                                                        1240   
      tstart = second(dummy) - tfirst - tmovst
      wallwalkst = time_()                                              cnoncray
      iedone = 0
      nummov = 0
      icycle = 1
      igroup = 1
      idslave = 0
      iemove = ie
      iestart = ie
      ieacc = ie                                                         1250   
      slavedone = .false.
      walkdone = .false.
      numvalues = (loc(lastvalues) - loc(vals))/lenreal                 cp4
      numquabl = 0                                                      cp4
cquas      numquabl = (loc(kgrid) - loc(quasblk))/lenreal               cp4
      numfdbl = 0                                                       cp4
cfd      if (dofdiag) numfdbl = numfdels                                cp4
cfd      if (do4clwr) numfdbl = num4clvals                              cp4
      numbuf4 = numvalues+nmdovrho+numquabl+numfdbl                     cp4
      lenstepfrmslv = lenstepstrt + numbuf4*lenreal4                     1260   
cMPI
      len8stepfrmslv = (lenstepfrmslv+7)/8
      len8steptoslv = (lensteptoslv+7)/8
      len8stepstrt = (lenstepstrt+7)/8
      len8stepblock = (lenstepblock+7)/8
      len8times = (lentimes+7)/8
cMPI
      if (master)  write (6, 393)  lenstepfrmslv, lenstepstrt,          cp4
     &  numbuf4, numvalues, nmdovrho, numquabl, numfdbl                 cp4
 393  format ('0Buffer from/to slave (bytes):', 2i6 /                    1270   
     &  ' buf4, values, rho, ...', 8i6 )                                cp4
      if ( numbuf4 .gt. maxbuf4 )  then                                 cp4
         write (0,*) ' Too much for BUF4!!:', maxbuf4, numbuf4,         cp4
     &      numvalues, nmdovrho, numquabl, numfdbl                      cp4
         call stop (' Too much for BUF4!!', numbuf4)                    cp4
      endif                                                             cp4
c                                                                       cp4
c     setup the buffering of results for difference calculations        cp4
c                                                                       cp4
      if ( tracsw )  then                                                1280   
         if ( master ) then                                             cp4
            do i = 1, maxnewes                                          cp4
               newie(i) = 0                                             cp4
            enddo                                                       cp4
            do i = 1, maxoldvals                                        cp4
               oldie(i) = 0                                             cp4
            enddo                                                       cp4
            koldval = 0                                                 cp4
            numnewes = 0                                                cp4
            maxnewesused = 0                                             1290   
            numoldval = 0                                               cp4
            maxoldvalsused = 0                                          cp4
         endif                                                          cp4
         newfull = .false.                                              cp4
         oldfull = .false.                                              cp4
         nstepbl8 = (lenstepfrmslv+lenreal-1)/lenreal                   cp4
         if (master)  write (6, 397) ntrac2, maxntrace, nstepbl8,       cp4
     &       maxdifstepblk                                              cp4
 397     format ( ' Buffers for difference calc:', 5x,                  cp4
     &     ' oldvals:', 2i5, 5x, ' step block:', 2i5 )                   1300   
         if (ntrac2 .gt. maxntrace                                      cp4
     &      .or.  nstepbl8 .gt. maxdifstepblk )  then                   cp4
            write (6,*) 'Too large for dif buffers', ntrac2,            cp4
     &         maxntrace, nstepbl8, maxdifstepblk, numbuf4, numvalues   cp4
            write (0,*) 'Too large for dif buffers', ntrac2,            cp4
     &         maxntrace, nstepbl8, maxdifstepblk, numbuf4, numvalues   cp4
            call stop ('Too large for dif buffers', ntrac2 )            cp4
         endif                                                          cp4
      endif                                                             cp4
c                                                                        1310   
      tim00 = second(tim00)
c
c
 400     tim0 = second(tim0)
         iedone = iedone+1
c
c     the master does the walk
c
         if ( .not. master ) go to 500                                  cp4
c                                                                        1320   
c     are we just finishing up the collection of results from the slaves
c
         if ( iedone .gt. nenerg )  then
            walkdone = .true.
            iedone = iedone - 1
            go to 450
         endif
         iemove = iemove + 1
c
         if ( tracsw )  then                                             1330   
c
            if ( .not. elimsw )  then
               read (walkun, end=930, err=930)  seednw, dseed,
     &            dlold, olddet, psinsq_old, wtjunk, weight,
     &            oldphiwt, oldpsijsq,
     &            ((xyzs(parwlkmap(i),j), i=1, npardm), j=1, 3)
     &            , ijordr                                              cnoncray
            else
               read (walkun, end=930, err=930)  seednw, dseed,
     &            dlold, olddet, psinsq_old, wtjunk, weight,             1340   
     &            oldphiwt, oldpsijsq, xyz_walk
     &            , ijordr                                              cnoncray
c
c     if we are eliminating nucleons, must recompute the c.m.
c
               do 429  ixyz = 1, 3
                  do 414 i = 1, numpar_walk
                     xyzs(parwlkmap(i),ixyz) = xyz_walk(i,ixyz)
 414              continue
                  xyzcm = 0                                              1350   
                  do 419  i = 1, numpar_all
                     xyzcm = xyzcm + xyzs(i,ixyz)
 419              continue
                  xyzcm = xyzcm/numpar_all
                  do 424  i = 1, numpar_all
                     xyzs(i,ixyz) = xyzs(i,ixyz) - xyzcm
 424              continue
 429           continue
            endif
c                                                                        1360   
c     if we are up to a cycle record, read and save it now              cp4
c                                                                       cp4
            if ( mod(iemove, ngroup) .eq. 0 )  then                     cp4
               koldval = koldval+1                                      cp4
               if ( koldval .gt. maxoldvals ) koldval = 1               cp4
               if ( oldie(koldval) .ne. 0 ) then                        cp4
                  write (0,*) ' *** error, old got full',               cp4
     &               koldval, oldie(koldval), iemove, numoldval         cp4
                  call stop ('old got full', koldval)                   cp4
               endif                                                     1370   
               difold(iedone) = koldval                                 cp4
               oldie(koldval) = iemove                                  cp4
               read (walkun, end=930, err=930)                          cp4
     &            ( oldblocks(i,koldval), i = 1, ntrac2 )               cp4
               numoldval = numoldval + 1                                cp4
               maxoldvalsused = max(maxoldvalsused, numoldval)          cp4
               knext = koldval+1                                        cp4
               if ( knext .gt. maxoldvals )  knext=1                    cp4
               oldfull = oldie(knext) .ne. 0                            cp4
               if (oldfull)  write(0,*) ' *** filled oldvals:',          1380   
     &            iemove, iedone, koldval                               cp4
            else                                                        cp4
               difold(iedone) = 0                                       cp4
            endif                                                       cp4
c
            numf = 0
            call getlr ( dl, psinsq, wavprod, f3c, ispcil, numf )
            ddl = dl - dl0
cray            call randordr
c                                                                        1390   
            if ( elimsw )  then
               bugops(10) = dl - dlold
               bugops(11) = exp(2*bugops(10))
               bugops(12) = 1
               do 439  i = 1, numdets
                  bugops(16+i) =
     &               ((real(dets(i)))**2+(imag(dets(i)))**2)/
     &               ((real(olddet(i)))**2+(imag(olddet(i)))**2)
                  bugops(12) = bugops(12)*bugops(16+i)
 439           continue                                                  1400   
               bugops(13) = bugops(12)*bugops(11)
               bugops(14) = bugops(13)*oldphiwt
c
c**********VERYSPECITAL FOR 16O 15N  !!!!!!
c
               pmiss = phsmis( xyz_walk(2,1), xyz_walk(2,2),
     &            xyz_walk(2,3) )
               bugops(15) = 1/pmiss
               bugops(16) = pmiss*bugops(14)
               oldpsijsq = oldpsijsq/pmiss                               1410   
            endif
c
         else
c
c     just reprocessing 4-body clusters?
c
cfd            if (do4clrd)  then
cfd               read (11) phiwt, t1s, d2s, t2s, v2s, d3s,
cfd     &            v3s, w3s, t3s, fd4cl
cfd               call foudiag (5)                                       1420   
cfd               go to 600
cfd            endif
c
c     make the nmoves random steps
c
            call movjas ( nmoves, nbd1, nbd2, nok, dl,
     2         psinsq, wavprod, f3c, weight, numf, ispcil )
            seedfl = dseed
            call randordr
c                                                                        1430   
            write (p4dbg,427) dl, psinsq, wavprod                       cdbcp4
 427        format ( ' MOVJAS:', 3g20.10  )                             cdbcp4
c
ctstphi      calltstphi
         endif
c
         flpmov = flpmov + numf
         tim1 = second(tim1)
         tmov = tmov + (tim1-tim0)
c                                                                        1440   
c     send the position to the slaves
c     gather all the stuff together.  We do this even in non-parallel
c     versions so that the same variable names may be used later
c
         job_bl = job
         ie_bl = iemove
         dl_bl = dl
         psinsq_bl = psinsq
         wavprod_bl = wavprod
         f3c_bl = f3c                                                    1450   
         weight_bl = weight
         dseed_bl = dseed
         seedfl_bl = seedfl
         oldphiwt_bl = oldphiwt
         oldpsijsq_bl = oldpsijsq
         do   i = 1, numdets
            dets_bl(i) = dets(i)
         enddo
         do   i = 1, 3*npardm
            xyzs_bl(i,1) = xyzs(i,1)                                     1460   
         enddo
         do   i = 1, 2*nmpair
            ijordr_bl(i,1) = ijordr(i,1)
         enddo
c
cccc     get the next slave; once all the slaves are started, we must
cccc     recover what he has done before sending a new position.
c
c     icode = -1 tells the slave it is done
c                                                                        1470   
 450     icode = 0
         if ( walkdone )  icode = -1
         icode_send = icode
c                                                                       cp4
c     copy what we are going to send to make room for what the slave retcp4
c                                                                       cp4
         do  i = 1, lenstepstrt_rl                                      cp4
            stepbl2(i) = stepbl(i)                                      cp4
         enddo                                                          cp4
c                                                                        1480   
  455    t11 = time_()                                                  cp4
         if ( .not. ( newfull .or. oldfull) )  then                     cp4
            idslave = anynode                                           cp4
ccMPIp4            call p4recv ( kp4stepblfrmslv, idslave, stepbl,
ccMPIp4     &            lenstepblock, igot, irc )
         call MPI_Recv_( stepbl, len8stepblock, MPI_DOUBLE, idslave,
     &                 kp4stepblfrmslv,  MPI_COMM_WORLD, status, irc )
         idslave = st_source
         igot = st_count
            tmastwait = tmastwait + time_()-t11                          1490   
         else                                                           cp4
c                                                                       cp4
c     when the buffers fill up, we must get stuff in the order          cp4
c     that we need it.                                                  cp4
c                                                                       cp4
            kienext = ieacc+1-iestart                                   cp4
            idslave = difproc(kienext)                                  cp4
            write (0,*) ' *** Getting from slave', idslave, kienext     cp4
ccMPIp4            call p4recv ( kp4stepblfrmslv, idslave, stepbl,
ccMPIp4     &            lenstepblock, igot, irc )                       1500   
         call MPI_Recv_( stepbl, len8stepblock, MPI_DOUBLE, idslave,
     &                 kp4stepblfrmslv,  MPI_COMM_WORLD, status, irc )
         idslave = st_source
         igot = st_count
            tmastfull = tmastfull + time_()-t11                         cp4
         endif                                                          cp4
         write (p4dbg,*) ' /stepbl/ back from slave:', idslave,         cdb1cp4
     &       idslave_bl, job, job_bl, icode, ie_bl, irc, len8stepblock, cdb1cp4
     &       igot !                                                     cdb1cp4
         if ( job .ne. job_bl )  then                                    1510   
            write (p4dbg,*) ' *** Slave', idslave, ' sent an old job',  cp4
     &        job, job_bl, icode, ie_bl                                 cp4
            icode = icode_send                                          cp4
            go to 455                                                   cp4
         endif                                                          cp4
         slavedone = icode .ge. 0                                       cp4
         idslave = idslave_bl                                           cp4
         idslave_bl2 = idslave                                          cp4
         if ( slavedone ) then                                          cp4
            numslavees(idslave) = numslavees(idslave)+1                  1520   
c                                                                       cp4
c     if we are tracing, just store the new values until needed         cp4
c                                                                       cp4
            if (tracsw)  then                                           cp4
               do knewie = 1, maxnewes                                  cp4
                  if ( newie(knewie) .eq. 0 )  go to 460                cp4
               enddo                                                    cp4
               write (0,*) ' *ERROR! no new slot', ie_bl, numnewes      cp4
               call stop ('no new slot', ie_bl)                         cp4
 460           numnewes = numnewes+1                                     1530   
               maxnewesused = max( maxnewesused, numnewes )             cp4
               difnew(ie_bl-iestart) = knewie                           cp4
               newie(knewie) = ie_bl                                    cp4
               write (0,*) ' saved new:', ie_bl, knewie, idslave_bl     cdbcp4
               do i = 1, nstepbl8                                       cp4
                  newblocks(i,knewie) = stepbl(i)                       cp4
               enddo                                                    cp4
               newfull = numnewes .ge. maxnewes-1                       cp4
               if ( newfull ) write (0,*)  ' *** filled new:',          cdbcp4
     &               knewie, ie_bl, maxnewes                             1540   
            else                                                        cp4
c                                                                       cp4
               do  i = 1, numvalues                                     cp4
                  vals(i) = buf4(i)                                     cp4
               enddo                                                    cp4
               ibuf = numvalues                                         cp4
               do  i = 1, nmdovrho                                      cp4
                  rhv(i) = buf4(ibuf+i)                                 cp4
               enddo                                                    cp4
               ibuf = ibuf+nmdovrho                                      1550   
cquas                  do  i = 1, numquabl                              cp4
cquas                     quasblk(i) = buf4(ibuf+i)                     cp4
cquas                  enddo                                            cp4
cquas                  ibuf = ibuf+numquabl                             cp4
cfd               if ( dofdiag )  then                                  cp4
cfd                  do  i = 1, numfdels                                cp4
cfd                     fdcount(i,1) = fdcount(i,1) + buf4(ibuf+i)      cp4
cfd                  enddo                                              cp4
cfd                  ibuf = ibuf+numfdels                               cp4
cfd               endif                                                  1560   
cfd               if ( do4clwr )  then                                  cp4
cfd                  do  i = 1, num4clvals                              cp4
cfd                     fd4cl(i) = buf4(ibuf+i)                         cp4
cfd                  enddo                                              cp4
cfd                  ibuf = ibuf+num4clvals                             cp4
cfd               endif                                                 cp4
            endif                                                       cp4
         endif                                                          cp4
c                                                                       cp4
c      send a new position or the stop signal to the slave               1570   
c                                                                       cp4
ccMPIp4         call p4send ( kp4stepbltoslv, idslave, stepbl2,
ccMPIp4     &         lenstepstrt, irc )
         call MPI_Send_( stepbl2, len8stepstrt, MPI_DOUBLE, idslave,
     &                 kp4stepbltoslv,  MPI_COMM_WORLD, irc )
         write (p4dbg,*) ' /stepbl/ sent to slave:', idslave,           cdb1cp4
     &          job, icode_send, iemove, len8stepstrt  , irc            cdb1cp4
         if ( walkdone ) then                                           cp4
            donesent(idslave) = .true.                                  cp4
         else                                                            1580   
            if ( tracsw )  then                                         cp4
               difproc(iedone) = idslave                                cp4
               difnew(iedone) = 0                                       cp4
            endif                                                       cp4
         endif                                                          cp4
c                                                                       cp4
c      if the walk is just starting, keep sending positions to slaves   cp4
c                                                                       cp4
         if ( .not. slavedone )  go to 400                              cp4
c                                                                        1590   
c     if we are doing a difference calculation then the values          cp4
c     must be used in sequence.  Is the next needed one now ready?      cp4
c                                                                       cp4
 480     if (tracsw)  then                                              cp4
            kiedone = ieacc+1-iestart                                   cp4
            if ( difnew(kiedone) .eq. 0 )  go to 400                    cp4
            knew = difnew(kiedone)                                      cp4
            write (0,*) ' averaging', kiedone, knew                     cdbcp4
            do i = 1, nstepbl8                                          cp4
               stepbl(i) = newblocks(i,knew)                             1600   
            enddo                                                       cp4
            newie(knew) = 0                                             cp4
            numnewes = numnewes-1                                       cp4
            do  i = 1, numvalues                                        cp4
               vals(i) = buf4(i)                                        cp4
            enddo                                                       cp4
            ibuf = numvalues                                            cp4
            do  i = 1, nmdovrho                                         cp4
               rhv(i) = buf4(ibuf+i)                                    cp4
            enddo                                                        1610   
            ibuf = ibuf+nmdovrho                                        cp4
cfd            if ( dofdiag )  then                                     cp4
cfd               do  i = 1, numfdels                                   cp4
cfd                  fdcount(i,1) = fdcount(i,1) + buf4(ibuf+i)         cp4
cfd               enddo                                                 cp4
cfd               ibuf = ibuf+numfdels                                  cp4
cfd            endif                                                    cp4
cfd            if ( do4clwr )  then                                     cp4
cfd               do  i = 1, num4clvals                                 cp4
cfd                  fd4cl(i) = buf4(ibuf+i)                             1620   
cfd               enddo                                                 cp4
cfd               ibuf = ibuf+num4clvals                                cp4
cfd            endif                                                    cp4
            newfull = .false.                                           cp4
         endif                                                          cp4
c                                                                       cp4
c     go on to accumulate the returned results                          cp4
c                                                                       cp4
         go to 600                                                      cp4
c                                                                        1630   
c     the slaves get the position or stop code                          cp4
c                                                                       cp4
 500     t11 = time_()                                                  cp4
ccMPIp4         call p4recv ( kp4stepbltoslv, masternode, stepbl,
ccMPIp4     &            lenstepstrt, igot, irc )
         call MPI_Recv_( stepbl, len8stepstrt, MPI_DOUBLE, masternode,
     &                 kp4stepbltoslv,  MPI_COMM_WORLD, status, irc )
         igot = st_count
         tslavwaitr = tslavwaitr + time_()-t11                          cp4
            write (p4dbg,*) ' /stepbl/ rcd by slave:', myid,             1640   
     &        idslave_bl, job, job_bl, icode, ie_bl, len8stepstrt,      cdb1cp4
     &        igot, irc                                                 cdb1cp4
         if ( job .ne. job_bl )  then                                   cp4
            write (p4dbg,*) ' *** Slave', myid, ' rcvd an old job',     cp4
     &        job, job_bl, icode, ie_bl                                 cp4
            go to 400                                                   cp4
         endif                                                          cp4
         if ( icode .eq. -1 )  then                                     cp4
            iedone = iedone-1                                           cp4
            t1 = second(dummy)                                           1650   
            tottim = t1 - tfirst                                        cp4
            tottme = t1 - tim00                                         cp4
            tmyid = myid                                                cp4
            tmclock = .01*mclock() - amclock1                           cnoncray
            delta = etime_(etime_struct)                                cnoncray
            tdsum = delta-delta1                                        cnoncray
            tdusr = usrtime-usr1                                        cnoncray
            tdsys = systime-sys1                                        cnoncray
ccMPIp4            call p4send ( kp4times, masternode, times,
ccMPIp4     &               lentimes, irc )                              1660   
         call MPI_Send_( times, len8times, MPI_DOUBLE, masternode,
     &                 kp4times,  MPI_COMM_WORLD, irc )
            call clock ( ttim2 )                                        cp4
            write (p4dbg,*) ttim2, ' slave, job:', myid, job,           cp4
     &          ' done after', iedone, ' energies'                      cp4
            jobdone = jobdone+iedone                                    cp4
            go to 1                                                     cp4
         else if ( icode .eq. -2 )  then                                cp4
            call clock ( ttim2 )                                        cp4
            write (p4dbg,*) ttim2, ' slave, job:', myid, job,            1670   
     &           ' done after no energies', iedone-1                    cp4
            go to 1                                                     cp4
         endif                                                          cp4
         ie = ie_bl                                                     cp4
c                                                                       cp4
         oldphiwt = oldphiwt_bl                                         cp4
         oldpsijsq = oldpsijsq_bl                                       cp4
         do  i = 1, numdets                                             cp4
            dets(i) = dets_bl(i)                                        cp4
         enddo                                                           1680   
         do  i = 1, 3*npardm                                            cp4
            xyzs(i,1) = xyzs_bl(i,1)                                    cp4
         enddo                                                          cp4
         do  i = 1, 2*nmpair                                            cp4
            ijordr(i,1) = ijordr_bl(i,1)                                cp4
         enddo                                                          cp4
c                                                                       cp4
c     recompute the jastrow value                                       cp4
c                                                                       cp4
         call getlr ( dl, psinsq, wavprod, f3c, ispcil, numf )           1690   
c!!                                                                     cp4
            write (p4dbg,527) dl, psinsq, wavprod                       cdbcp4
 527        format ( ' GETLR: ', 3g20.10 )                              cdbcp4
c!!                                                                     cp4
c                                                                       cp4
c     We add the slave stuff into 1-b kinetic energy so as to leave     cp4
c     the moving time as purely the master node time                    cp4
c                                                                       cp4
         flpkn1 = flpkn1 + numf                                         cp4
         tim1 = second(tim1)                                             1700   
         tkin1 = tkin1 + (tim1-tim0)                                    cp4
c                                                                       cp4
c     and then go on to the rest of the stuff                           cp4
c
c     setup the weight
c
            ddl = dl-dl0
            call accwt ( ddl, phibol, wavprod, f3c, weight_bl )
c
cfluc      if ( ispcil .eq. 1 )  then                                    1710   
cfluc         call getnfl
cfluc         go to 600
cfluc      endif
c
c     compute the one-body kinetic energy
c
            call kinet1 ( derstp, wavprod, f3c, numf )
crhok            if (dorhok)  then
crhok               call dorhk1 (nf)
crhok               numf = numf+nf                                       1720   
crhok            endif
cquas            call getqua1 ( wavprod )
ceep            call geteep1
cfc            if (dofc)  then
cfc               call getfcj (nf)
cfc               numf = numf+nf
cfc            endif
            flpkn1 = flpkn1 + numf
            tim2 = second(tim2)
            tkin1 = tkin1 + (tim2-tim1)                                  1730   
c
            call binrh1
c
c     compute the 2-body energy
c
            call twof6 ( derstp, numflp )                               cc2
            flpe2 = flpe2 + numflp                                      cc2
            tim4 = second(tim4)
            te2 = te2 + (tim4-tim2)
c                                                                        1740   
            call twof8 ( derstp, wavprod, f3c, numflp )                 cf8f6
            flp2f8 = flp2f8 + numflp                                    cf8f6
            tim3 = second(tim3)
            te2f8 = te2f8 + (tim3-tim4)
c
            call thrbod ( numflp )                                      cc3
            flpd3 = flpd3 + numflp                                      cc3
            tim4 = second(tim4)
            td3 = td3 + (tim4-tim3)
c                                                                        1750   
            if (doke .or. dorhok) then                                  cc3
               call thrkin ( derstp, numflp )                           cc3
               flpk3 = flpk3 + numflp                                   cc3
            endif                                                       cc3
            tim3 = second(tim3)
            tk3 = tk3 + (tim3-tim4)
c
            if (dov2 .or. dov3 .or. dorhop)  then                       cc3
               call thrpot ( numflp )                                   cc3
               flpv3 = flpv3 + numflp                                    1760   
            endif                                                       cc3
            tim2 = second(tlast)
            tv3 = tv3 + (tim2-tim3)
c
            call four ( derstp, tim2, flpd4, td4, flpk4, tk4,           cnoncray
     &   flpv4, tv4, tim4 )                                             cnoncray
c
cray            call fourbd ( numflp )                                  cc4
cray            flpd4 = flpd4 + numflp                                  cc4
cray            tim4 = second(tim4)                                      1770   
cray            td4 = td4 + (tim4-tim2)                                 cc4
crayc                                                                   cc4
cray            if (doke) then                                          cc4
cray               call fourkn ( derstp, numflp )                       cc4
cray               flpk4 = flpk4 + numflp                               cc4
cray            endif                                                   cc4
cray            tim2 = second(tim2)                                     cc4
cray            tk4 = tk4 + (tim2-tim4)                                 cc4
crayc                                                                   cc4
cray            if ( dov2 .or. dov3 .or. dorhop )  then                  1780   
cray               call fourpt ( numflp )                               cc4
cray               flpv4 = flpv4 + numflp                               cc4
cray            endif                                                   cc4
cray            tim4 = second(tim4)                                     cc4
cray            tv4 = tv4 + (tim4-tim2)                                 cc4
c
cfd            if (dofdiag)  call foudiag (2)
cfd            if (do4clwr)  call foudiag (4)
c
cdisc            if ( (doke .or. dov2) .and. .not. dorhok )  then        1790   
cdisc               call mkdisc ( u0, ua, numflp )
cdisc               flpdis = flpdis + numflp
cdisc            endif
            tlast = second(tlast)
            tdis = tdis + (tlast-tim4)
c                                                                       cp4
c     the slave sends its results back to the master                    cp4
c                                                                       cp4
         job_bl = job
         oldphiwt_bl = oldphiwt                                          1800   
         oldpsijsq_bl = oldpsijsq
         do i = 1, numvalues                                            cp4
            buf4(i) = vals(i)                                           cp4
         enddo                                                          cp4
         ibuf = numvalues                                               cp4
         do  i = 1, nmdovrho                                            cp4
            buf4(ibuf+i) = rhv(i)                                       cp4
         enddo                                                          cp4
         ibuf = ibuf+nmdovrho                                           cp4
cquas               do  i = 1, numquabl                                  1810   
cquas                  buf4(ibuf+i) = quasblk(i)                        cp4
cquas               enddo                                               cp4
cquas               ibuf = ibuf+numquabl                                cp4
cfd         if (dofdiag) then                                           cp4
cfd            do  i = 1, numfdels                                      cp4
cfd               buf4(ibuf+i) = fdcount(i,1)                           cp4
cfd               fdcount(i,1) = 0                                      cp4
cfd            enddo                                                    cp4
cfd            ibuf = ibuf+numfdels                                     cp4
cfd         endif                                                        1820   
cfd         if (do4clwr) then                                           cp4
cfd            do  i = 1, num4clvals                                    cp4
cfd               buf4(ibuf+i) = fd4cl(i)                               cp4
cfd            enddo                                                    cp4
cfd            ibuf = ibuf+numfdels                                     cp4
cfd         endif                                                       cp4
         t11 = time_()                                                  cp4
ccMPIp4         call p4send ( kp4stepblfrmslv, masternode, stepbl,
ccMPIp4     &         lenstepfrmslv, irc )
         call MPI_Send_( stepbl, len8stepfrmslv, MPI_DOUBLE, masternode, 1830   
     &                 kp4stepblfrmslv,  MPI_COMM_WORLD, irc )
         tslavwaits = tslavwaits + time_()-t11                          cp4
            write (p4dbg,*)  ' /stepbl/ sent by slave', myid, job,      cdb1cp4
     &         ie, irc                                                  cdb1cp4
c                                                                       cp4
c     the slave goes back to get another position                       cp4
c                                                                       cp4
         go to 400                                                      cp4
c                                                                       cp4
c                                                                        1840   
c     only the master gets this far
c
c     accumulate all the partial sums
c
 600     ieacc = ieacc+1
c
cfd               if ( do4clwr )  write (11) phiwt, t1s, d2s, t2s, v2s,
cfd     &            d3s, v3s, w3s, t3s, fd4cl
         call accumu ( ieacc, nbd1, nbd2, nok, dl_bl-dl0, psinsq_bl,
     1      wavprod_bl, f3c_bl, weight_bl, idbgsv )                      1850   
c
         if (dorho)  call getrho ( .false., xyzs_bl )
crhok            if (dorhok)  call getrhk
cquas            call getqua
cfc            if (dofc)  call getfc
c
         if (ndebug .ne. 0)  write (dbugun, 623)
     1      ( vals(mdebug(i)), i = 1, ndebug )
 623     format ( 1x, 9g14.5 )
c                                                                        1860   
c  <<<<<< the triplet counter is used only for diagnostics >>>>>>>>
c
ctrip            if (tripsw)  call triple ( numpar_all )
c
         if ( walksw )  then
            write (walkun)  dseed_bl, seedfl_bl, dl_bl, dets_bl,
     1         psinsq_bl, wtjunk, weight_bl, oldphiwt_bl,
     &         oldpsijsq_bl, xyzs_bl
     &           , ijordr_bl                                            cnoncray
         endif                                                           1870   
c
         if ( dbugsw )  then
            iie = mod(ieacc, 1000)
            write (6, 633) iie, dl_bl, psinsq_bl
 633        format ( 1x, i3, 6g12.3 )
         endif
c
c     is it time to make a subtotal?
c
         if ( igroup .lt. ngroup ) then                                  1880   
            igroup = igroup+1
            if (tracsw)  go to 480                                      cp4
            go to 400
         endif
         igroup = 1
c
         if (dbugsw)  write (6, *)  ' '
c
c     print subtotals sofar
c                                                                        1890   
         icyc = icyc + 1
c
c     close and open the walk unit to make sure it is all written.
c cnoncray - on RS6000  status='old' means append for write
c cray     = on Cray    we must use the non-standard position=
c     this is done before starting the new save unit; if it fails
c     the save unit will still refer to existing records in walkun.
c
         if (walksw)  then
            close ( unit=walkun )                                        1900   
            open ( unit=walkun, file=walkfl, err=945, iostat=iostat,    cnoncray
     &         status='old', form='unformatted' )                       cnoncray
cray            open ( unit=walkun, file=walkfl, err=945, iostat=iostat,
cray     &         status='old', form='unformatted', position='append' )
         endif
c
c     make sure we can read the old values before destroying the saveun
c
         if (tracsw)  then
c                                                                        1910   
c     if a parallel calc then we read the info sometime ago             cp4
c                                                                       cp4
            kold = difold(kiedone)                                      cp4
            write (0,*) ' Using oldvals:', kiedone, oldie(kold), kold   cdbcp4
            if ( kold .eq. 0 ) then                                     cp4
               write (6,*) ' *** Internal error in oldvals',            cp4
     &            kiedone, kold                                         cp4
               call stop ('Internal error in oldvals', kiedone)         cp4
            endif                                                       cp4
            do i = 1, ntrac2                                             1920   
               valold(i) = oldblocks(i,kold)                            cp4
            enddo                                                       cp4
            oldie(kold) = 0                                             cp4
            oldfull = .false.                                           cp4
            numoldval = numoldval-1                                     cp4
            go to 650                                                   cp4
c
            read ( walkun, end=930, err=930 )
     &         ( valold(i), i = 1, ntrac2 )
         endif                                                           1930   
c
c     the save file gets rewritten for every cycle
c     here we start it, then  prtacc, subrho, ..., add to it.
c     The position info is to allow the walk to be continued and hence
c     must be the most recent position from MOVJAS, not necesarily
c     the most recently returned slave position!
c
 650     if (savesw) then
            rewind saveun
            write (saveun)  savvr2, ieacc, icyc, tdat3, ttim3            1940   
            if ( .not. elimsw )  then
               write (saveun)  dseed, seedfl, dl, dets,
     1            psinsq, wtjunk, weight, xyzs, ijordr
            else
c!!!!
C!!!!!  what should we do with  ijordr_walk ????
c!!!!
               write (saveun)  dseed, seedfl, dl, dets,
     1            psinsq, wtjunk, weight, xyzs, ijordr_walk
            endif                                                        1950   
            iesave = ieacc
         endif
c
         call prtacc ( ieacc, icyc, nmoves, ngroup, .true., logun,
     1      logval, tfirst, ntrac2, walksw, savesw, walkun, saveun,
     &      ncycle )
c
         if (dorho)  call subrho ( ieacc, icyc, savesw, saveun )
crhok         if (dorhok)  call sbrhok ( ieacc, icyc, savesw, saveun )
cquas         call sbquas ( ieacc, icyc, savesw, saveun )                1960   
ceep         call sbeep ( ieacc, icyc, savesw, saveun )
cfc         if (dofc)  call sbfc ( ieacc, icyc, savesw, saveun )
c
cfd         if (dofdiag) write (saveun)  fdcount, fdsums
c
cfluc         if (logsw .and. ispcil .eq. 1)
cfluc     1       call shwnfl ( icycle, ieacc, tfirst )
c
c     close and open the save unit to make sure it is all written.
c     We open it append so that the final close when the program         1970   
c     terminates does not destroy it.
c
         if (savesw) then
            close ( unit=saveun )
            open ( unit=saveun, file=savefl, err=945, iostat=iostat,    cnoncray
     &          status='old', form='unformatted' )                      cnoncray
cray            open ( unit=saveun, file=savefl, err=945, iostat=iostat,
cray     &          status='old', form='unformatted', position='append'
         endif
c                                                                        1980   
c     finally, close and open the walk unit again because accumu
c     added the stuff for differences!
c
         if (walksw)  then
            close ( unit=walkun )
            open ( unit=walkun, file=walkfl, err=945, iostat=iostat,    cnoncray
     &         status='old', form='unformatted' )                       cnoncray
cray            open ( unit=walkun, file=walkfl, err=945, iostat=iostat,
cray     &         status='old', form='unformatted', position='append' )
         endif                                                           1990   
c
         if ( dbugsw )  write (6, *)  ' '
c
         if ( iabort .gt. 0 )  then
            write (6, *)  '0*** aborting at user request'
            go to 700
         endif
c
c     are we all done?
c                                                                        2000   
         if ( icycle .lt. ncycle )  then
            icycle = icycle + 1
            if (tracsw)  go to 480                                      cp4
            go to 400
         endif
c
 700  tottme = second(dummy) - tim00
      walle = time_() - wallwalkst
c
      write (6, 723)  ie_bl, ieacc, dseed*d2p31, dseed_bl*d2p31,         2010   
     &    dl, dl_bl, weight, weight_bl
 723  format ( '0final configuration = ', 2i7, 5x, 'seed =', 2f25.5 /
     &   5x, 'unbiased dl =', 2g20.10 /
     &   5x, 'weight =', 2g20.10 )
c
      if ( iwlksv .gt. 0 )  then
         close ( unit=walkun )
         close ( unit=saveun )
      endif
c                                                                        2020   
c     print total averages
c
      call prttot ( ieacc, icyc, nummov, idbgsv, ncycle )
c
      if (dorho)  call prtrho ( ieacc, icyc )
crhok      if (dorhok)  call prtrhk ( ieacc, icyc )
cquas      call prtqua ( ieacc, icyc )
ceep      call prteep ( ieacc, icyc )
cfc      if (dofc)  call prtfc ( ieacc, icyc )
cfd      if (dofdiag)  call foudiag (3)                                  2030   
c
c  <<<<<< the triplet counter is used only for diagnostics >>>>>>>>
c
ctrip      if (tripsw)  call prttrp ( numpar_all )
cfluc      if ( ispcil .eq. 1 )  call prtnfl
ctstphi      call tstphp ( ieacc )
c
c
c
      tim0 = second(dummy)                                               2040   
      iu = mod(isavin/10 000, 10 )
      if ( ( iu .le. 1  .and.  ( .not. tracsw )  .and. master .and.
     1  ( numpar_all .le. 30 ) )  .or.  iu .eq. 1  )
     1   call check  ( dl, derstp, mats, matins,                        xdet
     2     numav, numval  )
      tcheck = second(dummy) - tim0
c
      write (6, 801)
 801  format ('1' )                                                      2050   
c                                                                       cp4
c     collect the timing information from all the slaves                cp4
c     how many processors did somethin?                                 cp4
c                                                                       cp4
      numdid = 0                                                        cp4
      tmclock = 0                                                       cp4
      tdsum = 0                                                         cp4
      tdusr = 0                                                         cp4
      tdsys = 0                                                         cp4
      write (6, 802)                                                     2060   
 802  format ( // ' Slave    Number     wait for  ',                    cp4
     1     '   total_CPU    user_CPU    system_CPU   % system' /        cp4
     &    t10, 'of cases   send   recv')                                cp4
      do 819  idsl = 1, numnodes-1                                      cp4
         if ( donesent(idsl) ) then                                     cp4
            numdid = numdid+1                                           cp4
ccMPIp4            call p4recv ( kp4times, idsl, times2, lentimes, igot,
         call MPI_Recv_( times2, len8times, MPI_DOUBLE, idsl,
     &                 kp4times,  MPI_COMM_WORLD, status, irc )
         idslave = st_source                                             2070   
         igot = st_count
            tot_cpu = times2(2*numtim+5)                                cp4
            usr_cpu = times2(2*numtim+4)                                cp4
            sys_cpu = tot_cpu-usr_cpu                                   cp4
            sys_prcnt = 100*sys_cpu/(usr_cpu+1.e-5)                     cp4
            write (6, 813) idsl, numslavees(idsl),                      cp4
     &         times2(2*numtim+2), times2(2*numtim+3), tot_cpu,         cp4
     &         usr_cpu, sys_cpu, sys_prcnt                              cp4
 813        format ( i5, i11, f8.0, f7.0, 4f12.2 )                      cp4
            do 814  j = 1, 2*numtim+nump4tim-2                           2080   
               times(j) = times(j) + times2(j)                          cp4
 814        continue                                                    cp4
         else                                                           cp4
            write ( 6, 813 )  idsl, numslavees(idsl)                    cp4
            icode = -2                                                  cp4
            idslave_bl = idsl                                           cp4
ccMPIp4            call p4send ( kp4stepbltoslv, idslave_bl, stepbl,
ccMPIp4     &         lenstepstrt, irc )
         call MPI_Send_( stepbl, len8stepstrt, MPI_DOUBLE, idslave_bl,
     &                 kp4stepbltoslv,  MPI_COMM_WORLD, irc )            2090   
         endif                                                          cp4
 819  continue                                                          cp4
c                                                                       cp4
c
      timed = 0
      do 824  i = 1, nummaintim-4
         totflp = flop(i) + totflp
         timed = timed + times(i)
 824  continue
      tottim = tottim + second(dummy) - tfirst                           2100   
      tother = tottme - timed
      do 829  i = 1, numtim
         flops(i) = 1.e-6*flop(i)/max(times(i), .01)
         timper(i) = times(i)/max(iedone, 1)
 829  continue
      tjobcpu = tjobcpu + tottim
      tjobflp = tjobflp + totflp
      write (6, 833) cpuid, iedone,
     1   ( times(i), timper(i), 1.e-6*flop(i), flops(i),
     2     i = 1, nummaintim-2 )                                         2110   
 833  format ( '0', a40, 10x, 'Times for', i6, ' energies' /
     1   t34, 'CPU time', t55, 'MegaFltOps', 3x, 'MFLOP/sec' /
     1  t30, 'Total', 4x, 'per energy' //
     2  ' moving',             t25, f10.1, f13.6, f17.2, f11.3 /
     3  ' 1-body kinetic energy', t25, f10.1, f13.6, f17.2, f11.3 /
     4  ' 2-body f6 energy',   t25, f10.1, f13.6, f17.2, f11.3 /
     4  ' 2 body f8 energy   ',t25, f10.1, f13.6, f17.2, f11.3 /
     4  ' 3-body denominator', t25, f10.1, f13.6, f17.2, f11.3 /
     4  ' 3-body potential  ', t25, f10.1, f13.6, f17.2, f11.3 /
     4  ' 3-body k. e.      ', t25, f10.1, f13.6, f17.2, f11.3 /         2120   
     4  ' 4-body denominator', t25, f10.1, f13.6, f17.2, f11.3 /
     4  ' 4-body potential  ', t25, f10.1, f13.6, f17.2, f11.3 /
     4  ' 4-body k. e.      ', t25, f10.1, f13.6, f17.2, f11.3 /
     4  ' disconnected sums ', t25, f10.1, f13.6, f17.2, f11.3 /
     5  ' other loop time',    t25, f10.1, f13.6,
     5       t50, 2f3.0, t50, '      ' /
     5  ' total for energy',   t25, f10.1, f13.6, f17.2, f11.3,
     6      5x, 'FLOPs for all but other loop time' )
      write (6, 838)    tstart, tmovst, tcheck, tottim
 838  format (  '  + setup time of',   t25, f10.1 /                      2130   
     9  '  + initial moves',   t25, f10.1 /
     9  '  + check time   ',   t25, f10.1 /
     9  ' total time:',        t25, f10.1 )
c
ctim      write (6, 843) ( othertimname(i-nummaintim),
ctim     1    times(i), timper(i), 1.e-6*flop(i), flops(i),
ctim     2     i = nummaintim+1, numtim )
ctim 843  format ( / ' Detailed timings' /
ctim     1   t34, 'CPU time', t57, 'FLOPs', 5x, 'MFLOP/sec' /
ctim     1  t30, 'Total', 4x, 'per energy' //                            2140   
ctim     2  ( 1x, a20, t25, f10.1, f13.6, f17.2, f11.3 ) )
c
      elapsed = time_() - wallstart                                     cnoncray
      write (6, 863) elapsed, tottim/elapsed, 1.e-6*totflp/elapsed,     cnoncray
     &  100*tottim/(numnodes*elapsed),                                  cnoncray
     &  walle, tottme/walle, 1.e-6*totflp/walle,                        cnoncray
     &  100*tottme/(numnodes*walle)                                     cnoncray
 863  format (/ t15, 'Elapsed time   (CPU time)/Elapsed',               cnoncray
     &   3x, 'MFLOP/Elapsed', 3x, 'Efficency (%)' /                     cnoncray
     &   ' Total:' , t15, f10.0, f18.4, f18.2, f15.1 /                   2150   
     &   ' Energy loop:', t15, f10.0, f18.4, f18.2, f15.1 )             cnoncray
      write ( 6, 867) tstrtp4, tmastwait, tslavwaits,                   cp4
     &   tslavwaits/(numnodes-1), tslavwaitr, tslavwaitr/(numnodes-1)   cp4
 867  format ( / ' The MPI master startup time was', f5.0, ' seconds.' /cp4
     &  ' The master waited', f9.0,                                     cp4
     1   ' seconds for results from the slaves.' /                      cp4
     2  ' The slaves waited', f9.0, ' seconds,',                        cp4
     3     f9.2, ' per slave, for sending,' /                           cp4
     2  '               and', f9.0, ' seconds,',                        cp4
     3     f9.2, ' per slave, for receiving.' )                          2160   
      if (tracsw)  then                                                 cp4
         write (6, 868)  tmastfull, maxnewesused, maxnewes,             cp4
     &      maxoldvalsused, maxoldvals                                  cp4
 868     format ( ' The master also waited', f9.0, ' seconds ',         cp4
     &       'for results while a buffer was full.' /                   cp4
     &      ' The maximum number of new blocks saved was', i5,          cp4
     &         '; the limit was', i5 /                                  cp4
     &      ' The maximum number of old values saved was', i5,          cp4
     &         '; the limit was', i5 )                                  cp4
      endif                                                              2170   
      amc = .01*mclock() - amclock1                                     cnoncray
      delta = etime_(etime_struct) - delta1                             cnoncray
      write ( 6, 873) delta, amc, delta-amc,                            cnoncray
     &    100*(delta-amc)/(amc+1.e-5)                                   cnoncray
     1  , tdsum, tmclock, tdsum-tmclock,                                cp4
     &      100*(tdsum-tmclock)/(tmclock+1.e-5)                         cp4
 873  format ( / 13x, 'total_cpu      user_cpu     ',                   cnoncray
     &     'system_cpu    % system' /                                   cnoncray
     &  ' Master:', 4f14.2 /                                            cnoncray
     &  ' Slaves:', 4f14.2 )                                             2180   
c
      call date (thedat)
      call clock ( thetim )
      write (6, 917)  thedat, thetim
 917  format ( // ' Calculation ending on ', a8, ' at ', a8 )
      go to 1
c
      return
c
 930  write (6, 933) iemove, iesave                                      2190   
      if (logsw)  write (logun, 933) iemove, iesave
 933  format ( '0*** unexpected end of file on saved walk ',
     1     'file reading records for energy' , i7 /
     &   ' save unit last written for', i7, ' energies' )
      call stop ('unexpected end of file on saved walk', iemove)
 940  write (6, 943)  walkfl, savefl
 943  format( '0**** WALK or SAVE file not specified:' /
     &   ( 1x, a50 ) )
      call stop ('WALK or SAVE file not specified', 0)
 945  call stop ('Error opening or closing WALK or SAVE file',iostat)    2200   
 950  call stop ('error reading restart file record', ietape)
 955  write (6, 958) npxxxx, numpar_all
 958  format ('0**** input numpar_all =', i8, ' but must be', i8 )
      call stop ('bad numpar', npxxxx)
 960  call stop ('file 12 is not a saved walk file, or has an error',0)
 965  write (6, 967)  numpar_walk, i, isavin, iisvin, ngrptp, ngroup
 967  format ( '0*** conflict on file 12 for num nucleons:', 2i5,
     1  ' or options', 2i10, ' or group size:', 2i5 )
      call stop ('conflict on file 12', 0)
 970  call stop ('unexpected eof looking for averages',  ietape)         2210   
 975  call stop ('ran out of input deck', 0)
 980  call stop ('error or end of file reading save file', saveun)
c
c     end of input deck
c
 990  if ( frstsw )  go to 995
      firstparams = -1                                                  cp4
ccMPIp4      call p4brdcst( kp4params, firstparams, lenreal, irc )
           call MPI_BCAST_( firstparams, 1,  MPI_DOUBLE,
     &        masternode, MPI_COMM_WORLD, irc )                          2220   
      tjobflp = 1.e-6*tjobflp
      write (6, 993)  tjobcpu, tjobflp, tjobflp/tjobcpu
 993  format ( // ' Total JOB cpu time =', f9.1, ' seconds,',
     &  4x, ' Mega-flt-ops =', f11.0, 4x, 'MFLOPS =', f9.2 )
      elapsed = time_() - tjobwallst                                    cnoncray
      write (6, 994)  elapsed,  tjobflp/elapsed                         cnoncray
 994  format ( / ' Total JOB elased time =', f9.1, ' seconds,',         cnoncray
     &   4x, 'MFLOPS(elaspsed time) =', f9.2 )                          cnoncray
c
      if ( numdid .eq. numnodes-1 ) then                                 2230   
         write (0,*) ' doing MPI cleanup call'                          cp4
ccMPIp4         call p4cleanup
        call MPI_Finalize_( ierr )
         write (0,*) ' MPI_Finalize is done'                            cp4
      else                                                              cp4
         write (0,*) ' want to call p4error !!', numnodes-1-numdid      cp4
ccMPIp4         call p4error ( '*** Some nodes did nothing ',
ccMPIp4     &         numnodes-1-numdid )
        call MPI_Finalize_( ierr )
      endif                                                              2240   
c
      stop
c
 995  call stop ('missing input file', 5)
c
      end
      block data
C                                                                       version
C  ***********************************************************
C  *        16O    C. E. A   MPI  Version                    *
C  ***********************************************************
C
      implicit real*8 ( a-h, o-z )                                      implicit
c                                                                        2250   
      parameter ( nmpu = 4 ,  nmpd = 4 ,  nmnu = 4 ,  nmnd = 4 ,        numpar
     &   numpar_walk = 16 ,  numpar_all = 16 ,
     2   maxl = 1 , numrad = 2 , numstate = 8 , numphis = 8 ,
     &   numdets = 4 , maxparindet = 4 ,  lendets = 4 ,
     &   nmtype = 4 ,  nmexcg = 4 ,
     &   nm1cl = 1 ,  nm2cl = 4 ,  nm3cl = 5 ,   nm4cl = 11 ,
     &   mxdis2 = 1 ,  nm12cl = 1 ,  nm13cl = 1 ,  nm22cl = 1 ,
     &   nm1en = 4 ,  nm2en = 4 ,  nm3en = 4 ,   nm4en = 4 ,
     &   nm12en = 1 ,   nm13en = 1 ,   nm22en = 1 ,
     &   mx1pts = 16, mx2pts = 33, mx3pts = 257, mx4pts = 385,
     &   ln4wrk = 450 + 1  ,  nmstrip = ln4wrk-1 ,
     &   mx12pts = 1 ,  mx13pts = 1 ,  mx22pts = 1 )
      logical peqivn
      parameter ( peqivn = .true. )
      parameter ( nmrh1 = 3 ,   nmrh2 = 4+5 ,   maxrho = 51 )
      parameter ( mxgaus = 3 , ns1or2 = 2 , mxks = 1 )
      parameter ( mxspl1 = 201 )
      parameter ( numprot = nmpu+nmpd , numneut= nmnu+nmnd ,            genparam
     &  numpar = numprot+numneut ,
     4  matdim = maxparindet+1 ,
     2  npardm = numpar_all+1 ,  lnpar = numpar+1 , mxrorc = 2  )
      parameter (  nmpair_all = (numpar_all*(numpar_all-1))/2,
     &  nmpair_walk = (numpar_walk*(numpar_walk-1))/2 )
      parameter ( nmpair = (numpar*(numpar-1))/2 ,                      opsparam
     3   lnpair = nmpair+1 ,
     4   nmtrip = (nmpair*(numpar-2))/3 , lntrip = nmtrip+1 ,
     5   nmquad = (nmtrip*(numpar-3))/4 , lnquad = nmquad + 1 ,
     1   npairprot = (numprot*(numprot-1))/2 ,
     2   ntripprot = (npairprot*(numprot-2))/3 )
c                                                                       /onecom/
c     RADNODES, RADL, RADJ - nlj of each radial wavefunction;
C       RADJ is 2*j.  RADJ = -1 means no L.S is being used.
C   following define each "state" in PHIS
C     RADFUN - radial function to use, index to RADNODES, etc.
C     RADML - M value for Y(L,M)
C     RADMS - Spin indicator: 1 = up, 0 = down
C     RADFAC - Clebsch factor.
C   following define each single-particle wavefunction which is
C     a sum of the states defined above.
C     PHISTATE - index to the 1st element of RADFUN, etc.
C     PHINUM - number of states to add up.
C     PHILAST - index to the last element of RADFUN, etc.
C   following define each particle:
C     PARPHI - index to the PHISTATE, PHINUM array.
C     PARPORN - 0 = proton; 1 = neutron.
C     PARINOPER - .T. means the particle is an operator particle
c     PARWLKMAP - mapping array of where to particle positions read
c                 from walk file are to be stored.  Note extra last
c                 dummy element.
c     PARDET - determinant number (1 to NUMDETS)
c     PARINDET - index in the determ for the nucleon:
c           MATS(DETOFF(PARDET(i))+PARINDET(i))  is the row for i
C     IPAR(i) - index to the full (NUMPAR_ALL) particle array
C         for an operator particle (NUMPAR) index.
C     IIPAR(i)  index to the operator array for a particle in the
c         full array - 0 = not an operator nucleon
c     I1S(i) = i - list of indeices to t1s for the operator nucleons.
c         This is always just  1, 2, ..., numpar
c   following define the determinants ( 1 =< k =< NUMDETS )
c     NPARINDET(k) - number of nucleons in each determ = rank
c     DETPORN(k) - 0 = protons, 1 = neutrons
c     DETMS - 0 = spin down, 1 = spin up
c     DETPAR(j,k) - list of nucleons in the determ; values are to
c                   the full list of nucleons (numpar_all)
c     DETSTATE(j,k) - list of states (pointers to RADxxx) in each det
c     DETOFF(k) - (index-1) of first row of mats:
c               MATS(DETOFF(k)+i,j)   1 =< i,j =< NPARINDET(k)
c
      integer radnodes, radl, radj, radfun, radml, radms,
     1  phistate, phinum, philast, parphi, parporn, partyp, parwlkmap
      logical parinoper
      common /onecom/ radnodes(numrad), radl(numrad), radj(numrad),
     1  radfun(numstate), radml(numstate), radms(numstate),
     2  radfac(numstate),
     3  phistate(numphis), phinum(numphis), philast(numphis),
     3  parphi(numpar_all), parporn(numpar_all), parinoper(numpar_all),
     1  partyp(numpar_all), parwlkmap(numpar_walk+1),
     4  ipar(numpar), iipar(numpar_all), i1s(numpar)
      integer detporn, detms, detpar, detstate, pardet, detoff,
     &  parindet
      common /onecom/ nparindet(numdets), detporn(numdets),
     &  detms(numdets), detpar(maxparindet, numdets),
     &  detstate(maxparindet, numdets), pardet(numpar_all),
     &  detoff(numdets), parindet(numpar_all)
      common /onefloat/ clebdiv, ylmfac((maxl+1)**2),
     5  rmaxsq, u0scal, xdu0in, xdelu0, rmx0sq,
     6  radcub(4, numrad, mxspl1)
cnodt      parameter ( mx1pts = lnpar , mx2pts = lnpair ,               /classes
cnodt     &   mx3pts = lntrip , mx4pts = lnquad ,
cnodt     &   mx12pts = numpar*nmpair  ,  mx13pts = numpar*nmtrip  ,
cnodt     &   mx22pts = nmpair**2 )
      integer excgs
      integer*2 k1cls, k2cls, k3cls, k4cls, k12cls, k13cls, k22cls
      common /classes/  div1cl(nm1cl+1), div2cl(nm2cl+1),
     &    div3cl(nm3cl+1), div4cl(nm4cl+1),
     &    div12cl(nm12cl+1), div13cl(nm13cl+1), div22cl(nm22cl+1),
     &  cls1nm(nm1cl+1), cls2nm(nm2cl+1),
     &    cls3nm(nm3cl+1), cls4nm(nm4cl+1),
     &  rat1cl(nm1cl+1), rat2cl(nm2cl+1),
     &    rat3cl(nm3cl+1), rat4cl(nm4cl+1),
     &  div1pts(nm1cl+1), div2pts(nm2cl+1),
     &    div3pts(nm3cl+1), div4pts(nm4cl+1),
     &  excgs(nmtype, nmexcg),
     &  i1cls(nm1en, nm1cl), i2cls(2, nm2en, nm2cl),
     &    i3cls(3, nm3en, nm3cl), i4cls(4, nm4en, nm4cl ),
     &    i12cls(3, nm12en, nm12cl), i13cls(4, nm13en, nm13cl),
     &    i22cls(4, nm22en, nm22cl),
     &  nm1pts(nm1cl+1), nm2pts(nm2cl+1), nm3pts(nm3cl+1),
     &    nm4pts(nm4cl+1),
     &    nm12pts(nm12cl+1), nm13pts(nm13cl+1), nm22pts(nm22cl+1)
      common /classes/
     &  m21cl(2, nm2cl), m31cl(3, nm3cl), m32cl(3, nm3cl),
     &  m41cl(4, nm4cl), m42cl(6, nm4cl), m43cl(4, nm4cl),
     &  m312cl(3, nm3cl),
     &  m412cl(12, nm4cl), m413cl(4, nm4cl), m422cl(6, nm4cl),
     &  m121cl(nm12cl), m122cl(nm12cl),
     &  m131cl(nm13cl), m132cl(3,nm13cl), m133cl(nm13cl),
     &      m1312cl(3,nm13cl),
     &  m221cl(2,nm22cl), m222cl(2,nm22cl), m2212cl(2,nm22cl)
      common /classes/
     &  k1cls(mx1pts, nm1cl), k2cls(mx2pts, nm2cl),
     &    k3cls(mx3pts, nm3cl), k4cls(mx4pts, nm4cl),
     &    k12cls(mx12pts, nm12cl), k13cls(mx13pts, nm13cl),
     &    k22cls(mx22pts, nm22cl)
c
ceep>eepmxks
ceep>eepnumpar
ceep>/eepbl/
ceep>eepdata                                                             2260   
c
C                                                                       singleda
C     16O  with no L.S potential
C
      data
     1  radnodes / 0, 0 /,  radl / 0, 1 /,  radj / -1, -1 /,
     2  radfun / 1,  2, 2, 2,   1,  2, 2, 2 /,
     3  radml /  0, -1, 0, 1,   0, -1, 0, 1 /,
     4  radms /  4*1,           4*0 /,
     5  radfac / 8*1. /,  clebdiv / 1. /,
     6  phistate / 1, 2, 3, 4, 5, 6, 7, 8 /,
     7  phinum / 8*1 /,
     8  parphi  / 1, 2, 3, 4, 5, 6, 7, 8,   1, 2, 3, 4, 5, 6, 7, 8 /,
     9  parporn / 8*0,                      8*1 /,
     a  parinoper / 16*.true. /,
     b  parwlkmap / 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
     c              16, 17 /
      data
     &  detporn / 0, 0, 1, 1 /,  detms / 1, 0, 1, 0 /
c
c     16O, determinants, unsymetrized cluster expansion
c
      data
     &  partyp / 4*1 ,  4*2 ,  4*3 ,  4*4 /,
     &  excgs / 1, 2, 3, 4,      2, 1, 4, 3,
     &          3, 4, 1, 2,      4, 3, 2, 1  /
      end
c*id* stop
      subroutine stop ( msg, icode )
c
      character*(*) msg
c
      write ( 6, 13 )  msg, icode
      write ( 0, 23 )                                                    2270   
      write ( 0, 13 )  msg, icode
      write ( 0, 23 )
 13   format ( / 1x, 78('*') /
     &   ' *',20x, 'Stopping because of an error  !!', t79, '*' /
     &   ' * ', a, ' : ', i10,  t79, '*' /
     &   1x, 78('*') / )
 23   format ( '' )
c
ccMPIp4      call p4error ( msg, icode )
        call MPI_Finalize_( ierr )                                       2280   
      stop 9999
      end

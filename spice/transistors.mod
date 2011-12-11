* Transistors.  These are placeholders because I can't tell what the
* actual transistors were.  
*
* The CDC 6600 Training Manual (60147400), appendix B, describes the
* basic circuit properties.  It shows the "on" state of the transistor
* as having Ib = 1 mA, Ic = 10 mA, implying a beta of 10.  Note that
* the DD60 is an analog design that uses several different transistor
* types, so this is clearly a simplification, hopefully not too
* serious.
*
* The following models are taken from the Philips (NXP) library of
* small signal transistor SPICE models.  It's a bit of a search to look
* for something plausible.  On the one hand, the transistors in the
* 6600 and DD60 are probably fairly low performance by modern standards,
* but still they delivered logic circuits with 5 ns rise/fall times which
* is not shabby at all.
*
* The ones we're actually used are named "mpnp" and "mnpn"
*
*
*.MODEL QMMBT2222A NPN
.model mnpn NPN
+    IS=29.13E-15 
+    NF=992.6E-3 
+    ISE=9.652E-15 
+    NE=1.516  
+    BF=256.7  
+    IKF = 489.9E-3 
+    VAF = 80.99  
+    NR = 984.4E-3 
+    ISC = 320.3E-12 
+    NC = 1.608  
+    BR = 6.590  
+    IKR = 192.9E-3 
+    VAR = 101.2  
+    RB = 1.000  
+    IRB = 1.000E-3 
+    RBM = 1.000  
+    RE = 193.4E-3 
+    RC = 224.8E-3 
+    XTB = 0.000  
+    EG = 1.110  
+    XTI = 3.000  
+    CJE = 25.89E-12 
+    VJE = 689.1E-3 
+    MJE = 366.8E-3 
+    TF = 293.9E-12 
+    XTF = 71.78  
+    VTF = 20.00  
+    ITF = 4.797  
+    PTF = 0.000  
+    CJC = 10.11E-12 
+    VJC = 662.2E-3 
+    MJC = 416.0E-3 
+    XCJC = 0.5946  
+    TR = 320.0E-9 
+    CJS = 0.000  
+    VJS = 750.0E-3 
+    MJS = 333.0E-3 
+    FC = 938.8E-3
**
.MODEL QBFS19 NPN
+     IS=2.9320E-16
+     NF=0.9869
+     ISE=1.814E-15
+     NE=1.411
+     BF=84.24  
+     IKF=0.1702
+     VAF=130.5
+     NR=0.9848
+     ISC=3.133E-16
+     NC=0.9935
+     BR=2.933
+     IKR=9.000
+     VAR=4.860
+     RB=1
+     IRB=1E-06
+     RBM=1  
+     RE=0.8047
+     RC=0.9861
+     XTB=0
+     EG=1.11
+     XTI=3
+     CJE=1.922E-12  
+     VJE=0.6927
+     MJE=0.3045
+     TF=6.298E-10
+     XTF=113.6
+     VTF=5.391
+     ITF=0.4458
+     PTF=0
+     CJC=1.554E-12
+     VJC=0.3932
+     MJC=0.1891  
+     XCJC=0.1200
+     TR=1.00E-07
+     CJS=0
+     VJS=0.75
+     MJS=0.333
+     FC=0.9333
*
.MODEL QBF324 PNP
+     IS = 3.686E-16 
+     NF = 0.9966 
+     ISE = 2.604E-16 
+     NE = 1.263 
+     BF = 48.63 
+     IKF = 0.081 
+     VAF = 69 
+     NR = 1.01 
+     ISC = 9.693E-13 
+     NC = 1.56 
+     BR = 1.682 
+     IKR = 0.1 
+     VAR = 23 
+     RB = 18 
+     IRB = 3E-06 
+     RBM = 2 
+     RE = 0.4232 
+     RC = 1.5 
+     XTB = 0 
+     EG = 1.11 
+     XTI = 3 
+     CJE = 1.99E-12 
+     VJE = 0.7036 
+     MJE = 0.2976 
+     TF = 2.559E-10 
+     XTF = 4.5 
+     VTF = 6 
+     ITF = 0.1 
+     PTF = 0 
+     CJC = 3.103E-12 
+     VJC = 0.4209 
+     MJC = 0.3358 
+     XCJC = 0.0464 
+     TR = 3E-08 
+     CJS = 0 
+     VJS = 0.75 
+     MJS = 0.333 
+     FC = 0.9506
*
.MODEL QBC337-25 NPN
+    IS = 4.13E-14 
+    NF = 0.9822 
+    ISE = 3.534E-15 
+    NE = 1.35 
+    BF = 292.4 
+    IKF = 0.9 
+    VAF = 145.7 
+    NR = 0.982 
+    ISC = 1.957E-13 
+    NC = 1.3 
+    BR = 23.68 
+    IKR = 0.1 
+    VAR = 20 
+    RB = 60 
+    IRB = 0.0002 
+    RBM = 8 
+    RE = 0.1129 
+    RC = 0.25 
+    XTB = 0 
+    EG = 1.11 
+    XTI = 3 
+    CJE = 3.799E-11 
+    VJE = 0.6752 
+    MJE = 0.3488 
+    TF = 5.4E-10 
+    XTF = 4 
+    VTF = 4.448 
+    ITF = 0.665 
+    PTF = 90 
+    CJC = 1.355E-11 
+    VJC = 0.3523 
+    MJC = 0.3831 
+    XCJC = 0.455 
+    TR = 3E-08 
+    CJS = 0 
+    VJS = 0.75 
+    MJS = 0.333 
+    FC = 0.643
*
.MODEL BC327-25 PNP
+ IS=1.08E-13
+ NF=0.99
+ ISE=2.713E-14
+ NE=1.4
+ BF=385.7
+ IKF=0.3603
+ VAF=31.29
+ NR=0.9849
+ ISC=5.062E-13
+ NC=1.295
+ BR=20.57
+ IKR=0.054
+ VAR=11.62
+ RB=1
+ IRB=1E-06
+ RBM=0.5
+ RE=0.1415
+ RC=0.2623
+ XTB=0
+ EG=1.11
+ XTI=3
+ CJE=5.114E-11
+ VJE=0.8911
+ MJE=0.4417
+ TF=7.359E-10
+ XTF=1.859
+ VTF=3.813
+ ITF=0.4393
+ PTF=0
+ CJC=2.656E-11
+ VJC=0.62
+ MJC=0.4836
+ XCJC=0.459
+ TR=5.00E-08
+ CJS=0
+ VJS=0.75
+ MJS=0.333
+ FC=0.99
*
*
*.MODEL QPMBT2907A PNP
.model mpnp PNP
+    IS = 4.43E-14 
+    NF = 0.9912 
+    ISE = 1.088E-14 
+    NE = 1.778 
+    BF = 247 
+    IKF = 0.505 
+    VAF = 46.5 
+    NR = 0.9921 
+    ISC = 7.13E-15 
+    NC = 1.08 
+    BR = 17.69 
+    IKR = 0.06 
+    VAR = 14 
+    RB = 34 
+    IRB = 0.00015 
+    RBM = 2.5 
+    RE = 0.1092 
+    RC = 0.25 
+    XTB = 0 
+    EG = 1.11 
+    XTI = 3 
+    CJE = 3.37E-11 
+    VJE = 0.8967 
+    MJE = 0.4354 
+    TF = 4.9E-10 
+    XTF = 4 
+    VTF = 10 
+    ITF = 0.7 
+    PTF = 0 
+    CJC = 2.203E-11 
+    VJC = 0.9 
+    MJC = 0.4495 
+    XCJC = 0.6 
+    TR = 8E-09 
+    CJS = 0 
+    VJS = 0.75 
+    MJS = 0.333 
+    FC = 0.999
*

-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- QC module, rev B
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qcslice1 is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      tp_ya : out logicsig;
      y : out logicsig);

end qcslice1;
architecture gates of qcslice1 is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);


  u2 : inv port map (
    a => c,
    y => t2);


  u3 : g2 port map (
    a => t1,
    b => t2,
    y => tp_ya,
    y2 => y);



end gates;

use work.sigs.all;

entity qcslice2 is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      tp_ya : out logicsig;
      y : out logicsig;
      yb : out logicsig);

end qcslice2;
architecture gates of qcslice2 is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y2 => t1);

  tp_ya <= t1;

  u2 : g3 port map (
    a => t1,
    b => c,
    c => d,
    y => yb);


  u4 : g2 port map (
    a => t1,
    b => e,
    y => y);



end gates;

use work.sigs.all;

entity qc is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p7 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p21 : out logicsig;
      p24 : out logicsig);

end qc;
architecture gates of qc is
  component qcslice1
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      tp_ya : out logicsig;
      y : out logicsig);

  end component;

  component qcslice2
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      tp_ya : out logicsig;
      y : out logicsig;
      yb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;

begin -- gates
  u1 : qcslice1 port map (
    a => p6,
    b => p4,
    c => p8,
    tp_ya => a,
    y => p10);

  tp1 <= a;

  u2 : qcslice2 port map (
    a => p5,
    b => p3,
    c => a,
    d => f,
    e => a,
    tp_ya => e,
    y => p7,
    yb => p13);

  tp2 <= e;

  u3 : qcslice2 port map (
    a => p15,
    b => p19,
    c => b,
    d => d,
    e => b,
    tp_ya => f,
    y => p17,
    yb => p12);

  tp3 <= f;

  u4 : qcslice1 port map (
    a => p14,
    b => p16,
    c => p20,
    tp_ya => b,
    y => p11);

  tp4 <= b;

  u5 : qcslice1 port map (
    a => p25,
    b => p27,
    c => p23,
    tp_ya => c,
    y => p21);

  tp5 <= c;
  
  u6 : qcslice2 port map (
    a => p26,
    b => p28,
    c => c,
    d => e,
    e => c,
    tp_ya => d,
    y => p24,
    yb => p18);

  tp6 <= d;

end gates;


-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- RB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rb is
    port (
      p3 : in  logicsig;
      p4 : in  coaxsig;
      p5 : in  coaxsig;
      p17 : in  coaxsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      tp2 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      p2 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p10_tp3 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15_tp6 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end rb;
architecture gates of rb is
  component cxreceiver
    port (
      a : in  coaxsig;
      y : out logicsig);

  end component;

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

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal r3 : logicsig;
  signal t1 : logicsig;
  signal t1_tp1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p4,
    y => t1);


  u2 : rsflop port map (
    r => a,
    s => t1,
    q => d,
    qb => e);

  t1_tp1 <= d;

  u3 : cxreceiver port map (
    a => p5,
    y => t2);


  u4 : rsflop port map (
    r => a,
    s => t2,
    q => f,
    qb => g);

  tp2 <= f;

  u5 : cxreceiver port map (
    a => p17,
    y => t3);


  u6 : rsflop port map (
    r => a,
    s => r3,
    q => i);

  tp4 <= i;

  u7 : cxreceiver port map (
    a => p17,
    y => t4);


  u8 : rsflop port map (
    r => a,
    s => t4,
    q => b,
    qb => c);

  tp5 <= b;

  u9 : inv port map (
    a => p3,
    y => a);


  u10 : inv port map (
    a => p22,
    y => h);


  u11 : g4 port map (
    a => p24,
    b => p19,
    c => p26,
    d => p20,
    y => p15_tp6);


  u12 : inv port map (
    a => p24,
    y => t5);


  u13 : inv port map (
    a => p19,
    y => t6);


  u14 : inv port map (
    a => p26,
    y => t7);


  u15 : inv port map (
    a => p20,
    y => t8);


  u16 : g2 port map (
    a => t5,
    b => h,
    y => p21);


  u17 : g2 port map (
    a => t6,
    b => h,
    y => p23);


  u18 : g2 port map (
    a => t7,
    b => h,
    y => p27);


  u19 : g2 port map (
    a => t8,
    b => h,
    y => p28);


  u20 : g4 port map (
    a => c,
    b => e,
    c => i,
    d => g,
    y => p12);


  u21 : g3 port map (
    a => b,
    b => e,
    c => g,
    y => p13);


  u22 : g3 port map (
    a => c,
    b => e,
    c => f,
    y => p6);


  u23 : g3 port map (
    a => b,
    b => e,
    c => f,
    y => p11);


  u24 : g3 port map (
    a => c,
    b => d,
    c => g,
    y => p8);


  u25 : g3 port map (
    a => b,
    b => d,
    c => g,
    y => p10_tp3);


  u26 : g3 port map (
    a => c,
    b => d,
    c => f,
    y => p2);


  u27 : g3 port map (
    a => b,
    b => d,
    c => f,
    y => p14);



end gates;


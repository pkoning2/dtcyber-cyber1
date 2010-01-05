-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- HZ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hz is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p28 : out logicsig);

end hz;
architecture gates of hz is
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

  component g5
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g6
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;

begin -- gates
  u1 : g5 port map (
    a => p17,
    b => a,
    c => b,
    d => c,
    e => d,
    y => t1);

  tp2 <= t1;

  u2 : g6 port map (
    a => p9,
    b => a,
    c => b,
    d => c,
    e => d,
    f => e,
    y => t2);

  tp1 <= t2;

  u3 : g4 port map (
    a => p14,
    b => a,
    c => b,
    d => c,
    y => t3);


  u4 : g3 port map (
    a => p16,
    b => a,
    c => b,
    y => t4);


  u5 : g2 port map (
    a => p15,
    b => a,
    y => t5);


  u6 : inv port map (
    a => p18,
    y => t6);


  u7 : g6 port map (
    a => t1,
    b => t2,
    c => t3,
    d => t4,
    e => t5,
    f => t6,
    y => f,
    y2 => g);

  p20 <= g;
  tp4 <= f;

  u8 : g5 port map (
    a => p2,
    b => p3,
    c => p4,
    d => p5,
    e => p6,
    y => t7);

  p1 <= t7;
  tp3 <= t7;

  u9 : g2 port map (
    a => p12,
    b => f,
    y => t8);


  u10 : inv port map (
    a => p27,
    y => t9);


  u11 : rsflop port map (
    r => t9,
    s => t8,
    q => t10);

  tp5 <= t10;

  u12 : inv port map (
    a => t10,
    y => p28);


  u13 : g2 port map (
    a => p25,
    b => p26,
    y => p23,
    y2 => t11);

  tp6 <= t11;

  u14 : g2 port map (
    a => t11,
    b => g,
    y => t12);

  p19 <= t12;
  p21 <= t12;
  p22 <= t12;

  u15 : inv port map (
    a => t11,
    y => p24);


  u16 : inv port map (
    a => p11,
    y => a);


  u17 : inv port map (
    a => p13,
    y => b);


  u18 : inv port map (
    a => p8,
    y => c);


  u19 : inv port map (
    a => p7,
    y => d);


  u20 : inv port map (
    a => p10,
    y => e);



end gates;


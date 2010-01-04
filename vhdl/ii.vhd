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
-- II module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ii is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end ii;
architecture gates of ii is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

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
  signal t13 : logicsig;

begin -- gates
  u1 : g4 port map (
    a => p12,
    b => p28,
    c => p26,
    d => p24,
    y => t1);


  u2 : g2 port map (
    a => t1,
    b => d,
    y => t2);


  u3 : rsflop port map (
    r => t2,
    s => p17,
    q => t3,
    qb => e);

  tp4 <= t3;

  u4 : g2 port map (
    a => a,
    b => t3,
    y => p27);


  u5 : g2 port map (
    a => b,
    b => t3,
    y => p25);


  u6 : g2 port map (
    a => t3,
    b => c,
    y => p18);


  u7 : g2 port map (
    a => e,
    b => f,
    y => t4);

  p14 <= t4;
  tp5 <= t4;

  u8 : rsflop port map (
    r => t2,
    s => p11,
    q => t5,
    qb => f);

  tp2 <= t5;

  u9 : g2 port map (
    a => a,
    b => t5,
    y => p3);


  u10 : g2 port map (
    a => b,
    b => t5,
    y => p1);


  u11 : g2 port map (
    a => t5,
    b => c,
    y => p5);


  u12 : g2 port map (
    a => f,
    b => e,
    y => p16,
    y2 => t6);


  u13 : g2 port map (
    a => g,
    b => h,
    y => t7,
    y2 => t8);

  p15 <= t7;
  tp3 <= t7;

  u14 : g3 port map (
    a => t6,
    b => p7,
    c => t8,
    y2 => p10);


  u15 : g3 port map (
    a => p21,
    b => p12,
    c => p23,
    y => t9);


  u16 : g2 port map (
    a => t9,
    b => d,
    y => t10);


  u17 : rsflop port map (
    r => t10,
    s => p9,
    q => t11,
    qb => h);

  tp1 <= t11;

  u18 : inv port map (
    a => p13,
    y => t12);


  u19 : g2 port map (
    a => t11,
    b => t12,
    y => p8);


  u20 : rsflop port map (
    r => t10,
    s => p19,
    q => t13,
    qb => g);

  tp6 <= t13;

  u21 : g2 port map (
    a => t12,
    b => t13,
    y => p20);


  u22 : inv port map (
    a => p4,
    y => a);


  u23 : inv port map (
    a => p22,
    y => b);


  u24 : inv port map (
    a => p6,
    y => c);


  u25 : inv2 port map (
    a => p2,
    y2 => d);



end gates;


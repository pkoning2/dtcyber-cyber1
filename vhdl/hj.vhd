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
-- HJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hj is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p4 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig);

end hj;
architecture gates of hj is
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
  signal g : logicsig;
  signal h : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
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

begin -- gates
  u1 : g3 port map (
    a => b,
    b => p12,
    c => d,
    y2 => t1);


  u2 : g2 port map (
    a => p5,
    b => t1,
    y => t2);


  u3 : g3 port map (
    a => t1,
    b => k,
    c => p3,
    y => t3);

  p4 <= k;

  u4 : g3 port map (
    a => p7,
    b => t1,
    c => j,
    y => h);

  p9 <= j;
  tp3 <= h;
  tp5 <= j;

  u5 : g2 port map (
    a => t2,
    b => h,
    y => e);

  tp1 <= e;

  u6 : g2 port map (
    a => h,
    b => t3,
    y => g);

  tp2 <= g;


  u7 : g3 port map (
    a => t2,
    b => h,
    c => t3,
    y => t4,
    y2 => p10);

  p18 <= t4;

  u8 : g2 port map (
    a => t4,
    b => p2,
    y => p1,
    y2 => p19);


  u9 : g2 port map (
    a => g,
    b => p28,
    y => t5);


  u10 : g2 port map (
    a => p28,
    b => e,
    y => t6);


  u11 : g2 port map (
    a => t5,
    b => t6,
    y => l,
    y2 => t7);


  u12 : inv port map (
    a => t7,
    y => p21);


  u13 : rsflop port map (
    r => p27,
    s => t5,
    q => a,
    qb => b);

  tp6 <= a;

  u14 : g2 port map (
    a => c,
    b => a,
    y => p25);

  tp4 <= c;

  u15 : inv port map (
    a => a,
    y => p26);


  u16 : g2 port map (
    a => a,
    b => d,
    y => p24);


  u17 : rsflop port map (
    r => p27,
    s => t6,
    q => c,
    qb => d);


  u18 : g2 port map (
    a => b,
    b => d,
    y => p23);


  u19 : inv port map (
    a => p8,
    y => j);


  u20 : inv port map (
    a => p6,
    y => k);


  u21 : inv port map (
    a => h,
    y => t8);


  u22 : g2 port map (
    a => p16,
    b => t8,
    y => p11);


  u23 : g2 port map (
    a => t8,
    b => l,
    y => t9);


  u24 : g2 port map (
    a => t9,
    b => p14,
    y => t10);


  u25 : inv port map (
    a => p15,
    y => t11);


  u26 : g2 port map (
    a => t10,
    b => t11,
    y => p17);


  u27 : g2 port map (
    a => t10,
    b => p15,
    y => p13);



end gates;


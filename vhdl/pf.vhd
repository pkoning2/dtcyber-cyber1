-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- PF module rev C
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pf is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp6 : out logicsig;
      p9_p11_p13_tp2 : out logicsig;
      p10_p12_p14_tp3 : out logicsig;
      p17_p19_p21_tp5 : out logicsig;
      p18 : out logicsig;
      p20_p22_p24_tp4 : out logicsig);

end pf;
architecture gates of pf is
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
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal g : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal w : logicsig;
  signal x : logicsig;

begin -- gates
  u1 : inv port map (
    a => p2,
    y => j);


  u2 : g2 port map (
    a => j,
    b => p5,
    y2 => d);


  u3 : g2 port map (
    a => j,
    b => p26,
    y2 => g);


  u4 : g2 port map (
    a => j,
    b => p4,
    y2 => k);


  u5 : g3 port map (
    a => j,
    b => p28,
    c => p27,
    y2 => c);


  u6 : g3 port map (
    a => a,
    b => e,
    c => c,
    y => w);

  p17_p19_p21_tp5 <= w;
  tp1 <= a;

  u7 : g3 port map (
    a => a,
    b => e,
    c => d,
    y => x);

  p9_p11_p13_tp2 <= x;

  u8 : g2 port map (
    a => i,
    b => g,
    y => p20_p22_p24_tp4);


  u9 : g2 port map (
    a => i,
    b => k,
    y => p10_p12_p14_tp3);


  u10 : inv2 port map (
    a => p3,
    y => i,
    y2 => a);


  u11 : inv port map (
    a => p1,
    y => e);


  u12 : g2 port map (
    a => x,
    b => w,
    y => t3);


  u13 : inv2 port map (
    a => p15,
    y2 => t4);


  u14 : g2 port map (
    a => t3,
    b => t4,
    y => t5);


  u15 : inv port map (
    a => p16,
    y => t6);


  u16 : rsflop port map (
    r => t6,
    s => t5,
    q => t7);

  tp6 <= t7;

  u17 : inv port map (
    a => t7,
    y => p18);



end gates;


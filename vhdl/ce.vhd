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
-- CE module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ce is
    port (
      p4 : in  logicsig;
      p5 : in  logicsig;
      p14 : in  logicsig;
      p18 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      tp3 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p6 : out logicsig;
      p7_tp1 : out logicsig;
      p8_tp2 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12_tp5 : out logicsig;
      p13_tp4 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end ce;
architecture gates of ce is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p4,
    y => a,
    y2 => i);

  tp3 <= a;

  u2 : g2 port map (
    a => p22,
    b => a,
    y => b);

  p8_tp2 <= b;

  u3 : inv2 port map (
    a => b,
    y2 => p1);


  u4 : g2 port map (
    a => p21,
    b => a,
    y => c);

  p7_tp1 <= c;

  u5 : inv2 port map (
    a => c,
    y2 => p2);


  u6 : g2 port map (
    a => p5,
    b => a,
    y => d);

  p6 <= d;

  u7 : inv2 port map (
    a => d,
    y2 => p3);


  u8 : inv2 port map (
    a => a,
    y => j,
    y2 => p9);

  p19 <= j;

  u9 : inv port map (
    a => p23,
    y => e);

  tp6 <= e;

  u10 : g3 port map (
    a => p25,
    b => i,
    c => e,
    y => f);

  p11 <= f;

  u11 : inv2 port map (
    a => f,
    y2 => p26);


  u12 : g3 port map (
    a => p24,
    b => e,
    c => i,
    y => g);

  p12_tp5 <= g;

  u13 : inv2 port map (
    a => g,
    y2 => p27);


  u14 : g3 port map (
    a => p18,
    b => e,
    c => i,
    y => h);

  p13_tp4 <= h;

  u15 : inv2 port map (
    a => h,
    y2 => p28);


  u16 : g2 port map (
    a => e,
    b => i,
    y => t1,
    y2 => p15);

  p16 <= t1;

  u17 : g2 port map (
    a => t1,
    b => p14,
    y => p17);


  u18 : g2 port map (
    a => p14,
    b => j,
    y => p10);



end gates;


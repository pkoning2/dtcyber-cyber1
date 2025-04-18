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
-- AP module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ap is
    port (
      p3 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p22 : in  logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6_tp1 : out logicsig;
      p7_tp2 : out logicsig;
      p8_tp3 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p17_tp5 : out logicsig;
      p18_tp4 : out logicsig;
      p19_tp6 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end ap;
architecture gates of ap is
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
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal q : logicsig;
  signal r : logicsig;
  signal s : logicsig;
  signal t3 : logicsig;
  signal t5 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => a,
    b => d,
    c => e,
    y => p2,
    y2 => p1);

  p4 <= a;
  p9 <= e;

  u2 : inv2 port map (
    a => p13,
    y => c,
    y2 => d);

  p14 <= c;

  u3 : inv2 port map (
    a => p11,
    y => e,
    y2 => f);


  u4 : inv2 port map (
    a => p22,
    y => g,
    y2 => h);


  u5 : inv2 port map (
    a => p16,
    y => s,
    y2 => j);


  u6 : inv2 port map (
    a => p15,
    y => k,
    y2 => l);


  u7 : g2 port map (
    a => h,
    b => s,
    y => p28);


  u8 : g3 port map (
    a => h,
    b => j,
    c => k,
    y => q);

  p25 <= q;

  u9 : g2 port map (
    a => h,
    b => j,
    y => p27);


  u10 : g3 port map (
    a => a,
    b => d,
    c => f,
    y => p6_tp1);


  u11 : g3 port map (
    a => b,
    b => c,
    c => e,
    y => p7_tp2,
    y2 => t3);


  u12 : g3 port map (
    a => h,
    b => t3,
    c => q,
    y2 => p10);


  u13 : g3 port map (
    a => b,
    b => c,
    c => f,
    y => p5,
    y2 => p12);


  u14 : g4 port map (
    a => a,
    b => c,
    c => e,
    d => r,
    y => p8_tp3);


  u15 : g3 port map (
    a => g,
    b => s,
    c => k,
    y => p24);


  u16 : g3 port map (
    a => g,
    b => j,
    c => l,
    y => t5,
    y2 => p18_tp4);

  p21 <= t5;

  u17 : g3 port map (
    a => g,
    b => j,
    c => k,
    y => t7);

  p23 <= t7;

  u18 : g2 port map (
    a => t5,
    b => t7,
    y => t8);

  p17_tp5 <= t8;

  u19 : g2 port map (
    a => g,
    b => s,
    y => r);

  p26 <= r;

  u20 : g2 port map (
    a => t7,
    b => r,
    y => p19_tp6);


  u21 : inv2 port map (
    a => p3,
    y => a,
    y2 => b);



end gates;


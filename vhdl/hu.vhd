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
-- HU module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hu is
    port (
      p2 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p21 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p6 : out logicsig;
      p8_tp1 : out logicsig;
      p9_tp2 : out logicsig;
      p13 : out logicsig;
      p14_tp5 : out logicsig;
      p16_tp6 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end hu;
architecture gates of hu is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal f : logicsig;
  signal h : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p27,
    y => a,
    y2 => p26);

  p28 <= a;

  u2 : inv2 port map (
    a => p25,
    y => b,
    y2 => p24);

  p23 <= b;

  u3 : inv2 port map (
    a => p21,
    y => p22,
    y2 => c);

  p19 <= c;

  u4 : inv2 port map (
    a => p15,
    y => p13,
    y2 => d);

  p16_tp6 <= d;

  u5 : inv2 port map (
    a => p17,
    y => p20,
    y2 => f);

  p18 <= f;

  u6 : inv2 port map (
    a => p2,
    y => t1,
    y2 => h);

  p1 <= h;

  u7 : inv2 port map (
    a => p5,
    y => p6,
    y2 => k);

  p3 <= k;

  u8 : g2 port map (
    a => t1,
    b => k,
    y => j);


  u9 : inv port map (
    a => p11,
    y => l);


  u10 : g4 port map (
    a => a,
    b => d,
    c => f,
    d => j,
    y => t2);


  u11 : g2 port map (
    a => l,
    b => t2,
    y => t3);


  u12 : g2 port map (
    a => t3,
    b => p12,
    y => p14_tp5);


  u13 : g3 port map (
    a => b,
    b => d,
    c => h,
    y => t5);


  u14 : g2 port map (
    a => l,
    b => t5,
    y => t6);


  u15 : g2 port map (
    a => t6,
    b => p10,
    y => p9_tp2);


  u16 : g3 port map (
    a => c,
    b => f,
    c => h,
    y => t8);


  u17 : g2 port map (
    a => l,
    b => t8,
    y => t9);


  u18 : g2 port map (
    a => t9,
    b => p7,
    y => p8_tp1);



end gates;


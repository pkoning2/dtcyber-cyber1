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
-- NG module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ng is
    port (
      p5 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p11_tp3 : out logicsig;
      p13_tp4 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end ng;
architecture gates of ng is
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
  signal k : logicsig;
  signal l : logicsig;
  signal m : logicsig;
  signal n : logicsig;
  signal o : logicsig;
  signal p : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => j,
    b => a,
    c => e,
    y => t1);

  tp2 <= t1;

  u2 : g2 port map (
    a => i,
    b => t1,
    y => p4,
    y2 => p6);


  u3 : g2 port map (
    a => o,
    b => l,
    y => t2);

  tp1 <= o;

  u4 : g2 port map (
    a => t2,
    b => k,
    y => t3,
    y2 => t4);

  tp6 <= t3;

  u5 : g3 port map (
    a => m,
    b => t3,
    c => p,
    y => p25);


  u6 : g2 port map (
    a => c,
    b => g,
    y => i);


  u7 : g2 port map (
    a => d,
    b => h,
    y => j);

  p27 <= h;

  u8 : g2 port map (
    a => i,
    b => j,
    y => t5,
    y2 => m);

  tp5 <= t5;

  u9 : g3 port map (
    a => t4,
    b => p,
    c => t5,
    y => p22);


  u10 : g2 port map (
    a => p8,
    b => p12,
    y => a,
    y2 => b);


  u11 : g2 port map (
    a => p24,
    b => p26,
    y => c,
    y2 => d);


  u12 : inv2 port map (
    a => p18,
    y => e,
    y2 => f);

  p17 <= f;

  u13 : inv2 port map (
    a => p28,
    y => g,
    y2 => h);


  u14 : inv port map (
    a => p20,
    y => p);


  u15 : g3 port map (
    a => p5,
    b => p10,
    c => p14,
    y => o,
    y2 => t10);


  u16 : g3 port map (
    a => n,
    b => o,
    c => p,
    y => p1);


  u17 : g2 port map (
    a => a,
    b => e,
    y => k);


  u18 : g2 port map (
    a => b,
    b => f,
    y => l);


  u19 : g2 port map (
    a => k,
    b => l,
    y => t11,
    y2 => n);

  p11_tp3 <= t11;

  u20 : g3 port map (
    a => t10,
    b => t11,
    c => p,
    y => p16);


  u21 : g2 port map (
    a => j,
    b => l,
    y => p13_tp4);



end gates;


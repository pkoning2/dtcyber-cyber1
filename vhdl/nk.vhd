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
-- NK module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nk is
    port (
      p5 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
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
      p23 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end nk;
architecture gates of nk is
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
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t10 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => e,
    y => k);


  u2 : g2 port map (
    a => b,
    b => f,
    y => l);


  u3 : g2 port map (
    a => k,
    b => l,
    y => t1,
    y2 => n);

  p11_tp3 <= t1;

  u4 : inv port map (
    a => p14,
    y => t2);


  u5 : g3 port map (
    a => p5,
    b => t2,
    c => p10,
    y => o,
    y2 => t3);

  tp1 <= o;

  u7 : g3 port map (
    a => p,
    b => t1,
    c => t3,
    y => p16);


  u8 : g3 port map (
    a => n,
    b => o,
    c => p,
    y => p1);


  u9 : g2 port map (
    a => o,
    b => l,
    y => t4);


  u10 : g2 port map (
    a => t4,
    b => k,
    y => t5,
    y2 => t6);

  tp6 <= t5;

  u11 : g3 port map (
    a => p,
    b => t5,
    c => m,
    y => p25);


  u12 : g2 port map (
    a => c,
    b => g,
    y => i);


  u13 : g2 port map (
    a => d,
    b => h,
    y => j);


  u14 : g2 port map (
    a => i,
    b => j,
    y => t7,
    y2 => m);

  tp5 <= t7;

  u15 : g3 port map (
    a => t6,
    b => t7,
    c => p,
    y => p22);


  u16 : g2 port map (
    a => j,
    b => l,
    y => p13_tp4);


  u17 : g2 port map (
    a => p8,
    b => p12,
    y => a,
    y2 => b);


  u18 : g2 port map (
    a => p24,
    b => p26,
    y => c,
    y2 => d);


  u19 : inv2 port map (
    a => p18,
    y => e,
    y2 => f);

  p17 <= f;

  u20 : inv2 port map (
    a => p28,
    y => g,
    y2 => h);

  p27 <= h;

  u21 : inv port map (
    a => p20,
    y => p);


  u22 : g2 port map (
    a => p21,
    b => p19,
    y => p23);


  u23 : g3 port map (
    a => j,
    b => a,
    c => e,
    y => t10);

  tp2 <= t10;

  u24 : g2 port map (
    a => t10,
    b => i,
    y => p4,
    y2 => p6);



end gates;


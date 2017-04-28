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
-- MU module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mu is
    port (
      p5 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p14_tp1 : out logicsig;
      tp2 : out logicsig;
      tp4 : out logicsig;
      p1_p2_p3_p4_p5 : out logicsig;
      p10 : out logicsig;
      p11_p13_tp3 : out logicsig;
      p12 : out logicsig;
      p20_p22_tp5 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end mu;
architecture gates of mu is
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
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p21,
    y => i);


  u2 : inv port map (
    a => p18,
    y => j);


  u3 : inv port map (
    a => p15,
    y => k);


  u4 : inv port map (
    a => p25,
    y => l);


  u5 : g3 port map (
    a => j,
    b => l,
    c => p17,
    y => h);

  tp4 <= h;

  u6 : inv port map (
    a => p17,
    y => a);


  u7 : inv port map (
    a => p9,
    y => e);


  u8 : g2 port map (
    a => p9,
    b => i,
    y => b);


  u9 : g2 port map (
    a => j,
    b => p23,
    y => f);


  u10 : g3 port map (
    a => p23,
    b => i,
    c => j,
    y => c);


  u11 : g4 port map (
    a => h,
    b => e,
    c => f,
    d => g,
    y => t2,
    y2 => t1);

  p14_tp1 <= t2;

  u12 : inv port map (
    a => t1,
    y => p11_p13_tp3);


  u13 : g2 port map (
    a => t2,
    b => p5,
    y => p12);


  u14 : g3 port map (
    a => p5,
    b => t2,
    c => p7,
    y => p10);


  u15 : g2 port map (
    a => k,
    b => p16,
    y => g);


  u17 : g3 port map (
    a => p16,
    b => i,
    c => k,
    y => d);


  u18 : g4 port map (
    a => d,
    b => c,
    c => b,
    d => a,
    y => t3,
    y2 => t4);

  p28 <= t3;

  u19 : inv port map (
    a => t4,
    y => p20_p22_tp5);


  u20 : g3 port map (
    a => p19,
    b => t3,
    c => p27,
    y => p24);


  u21 : g2 port map (
    a => p27,
    b => t3,
    y => p26);


  u22 : inv2 port map (
    a => p8,
    y => tp2,
    y2 => p1_p2_p3_p4_p5);



end gates;


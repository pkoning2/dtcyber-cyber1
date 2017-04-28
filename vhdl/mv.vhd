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
-- MV module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mv is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p28 : in  logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      p2_p1_p4_p6_p8 : out logicsig;
      p9_p11_tp2 : out logicsig;
      p10 : out logicsig;
      p12_tp1 : out logicsig;
      p18 : out logicsig;
      p26_p27_tp6 : out logicsig);

end mv;
architecture gates of mv is
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
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => p24,
    y => g);


  u2 : g3 port map (
    a => a,
    b => b,
    c => p21,
    y => h);


  u3 : g3 port map (
    a => a,
    b => c,
    c => p19,
    y => i);


  u4 : g4 port map (
    a => a,
    b => b,
    c => d,
    d => p22,
    y => j);


  u5 : inv port map (
    a => p24,
    y => t4);


  u6 : g2 port map (
    a => p21,
    b => b,
    y => t3);


  u7 : g3 port map (
    a => p22,
    b => b,
    c => d,
    y => t1);


  u8 : g2 port map (
    a => p19,
    b => c,
    y => t2);


  u9 : g2 port map (
    a => p20,
    b => p14,
    y => p12_tp1);


  u10 : inv port map (
    a => p15,
    y => a);


  u11 : inv port map (
    a => p25,
    y => b);


  u12 : inv port map (
    a => p23,
    y => c);


  u13 : inv port map (
    a => p7,
    y => d);


  u14 : inv port map (
    a => p28,
    y => t5);


  u15 : g6 port map (
    a => g,
    b => k,
    c => h,
    d => i,
    e => t5,
    f => j,
    y => p26_p27_tp6);


  u16 : g3 port map (
    a => p16,
    b => p17,
    c => c,
    y => k);


  u17 : g5 port map (
    a => k,
    b => t1,
    c => t2,
    d => t3,
    e => t4,
    y => t6,
    y2 => t7);

  tp4 <= t6;

  u18 : g3 port map (
    a => p13,
    b => t6,
    c => p5,
    y => p18);


  u19 : g2 port map (
    a => p5,
    b => t6,
    y => p10);


  u20 : inv port map (
    a => t7,
    y => p9_p11_tp2);


  u21 : inv2 port map (
    a => p3,
    y => tp3,
    y2 => p2_p1_p4_p6_p8);



end gates;


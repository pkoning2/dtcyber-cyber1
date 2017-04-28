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
-- TS module (variant of MM)
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ts is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p27 : in  logicsig;
      tp2 : out logicsig;
      p1_tp3 : out logicsig;
      p3_tp1 : out logicsig;
      p6 : out logicsig;
      p10_p14 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p21_tp4 : out logicsig;
      p22 : out logicsig;
      p23_p25 : out logicsig;
      p24_tp5 : out logicsig;
      p26_tp6 : out logicsig;
      p28 : out logicsig);

end ts;
architecture gates of ts is
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
  signal c : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => p16,
    b => j,
    c => p9,
    y => p12);

  p10_p14 <= j;

  u2 : inv2 port map (
    a => p9,
    y2 => f);


  u3 : inv2 port map (
    a => p19,
    y => g,
    y2 => t4);


  u4 : g3 port map (
    a => a,
    b => p2,
    c => h,
    y => t1);


  u5 : g2 port map (
    a => c,
    b => h,
    y => t2);


  u7 : g4 port map (
    a => t1,
    b => t2,
    c => t3,
    d => i,
    y => p1_tp3);

  tp2 <= i;

  u8 : g3 port map (
    a => p18,
    b => j,
    c => p4,
    y => p6);


  u9 : inv2 port map (
    a => p4,
    y2 => a);


  u10 : inv2 port map (
    a => p7,
    y => c,
    y2 => t9);


  u11 : g3 port map (
    a => p8,
    b => j,
    c => p17,
    y => p11);


  u12 : inv2 port map (
    a => p17,
    y2 => h);


  u13 : inv2 port map (
    a => p20,
    y => e,
    y2 => t3);


  u14 : inv2 port map (
    a => p15,
    y => i,
    y2 => j);


  u16 : g2 port map (
    a => f,
    b => e,
    y => t5);


  u17 : g3 port map (
    a => f,
    b => h,
    c => c,
    y => t6);


  u18 : g3 port map (
    a => t4,
    b => t5,
    c => t6,
    y => p26_tp6);


  u19 : inv port map (
    a => p5,
    y => t7);


  u20 : g2 port map (
    a => t7,
    b => i,
    y => p21_tp4);


  u21 : g2 port map (
    a => p5,
    b => a,
    y => t8);


  u23 : g3 port map (
    a => i,
    b => t8,
    c => t9,
    y => p3_tp1);


  u24 : inv2 port map (
    a => p27,
    y => p23_p25,
    y2 => p28);


  u25 : g3 port map (
    a => f,
    b => a,
    c => h,
    y => p22,
    y2 => p24_tp5);


  p13 <= '0';

end gates;


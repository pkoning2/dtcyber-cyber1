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
-- HV module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hv is
    port (
      p1 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p28 : out logicsig);

end hv;
architecture gates of hv is
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
  u1 : inv2 port map (
    a => p12,
    y2 => a);

  p10 <= a;

  u2 : inv2 port map (
    a => p14,
    y2 => c);


  u3 : inv port map (
    a => p18,
    y => t1);

  p16 <= t1;

  u4 : g2 port map (
    a => j,
    b => t1,
    y => g);


  u5 : g3 port map (
    a => c,
    b => p7,
    c => g,
    y => tp1,
    y2 => p8);


  u6 : g3 port map (
    a => a,
    b => c,
    c => h,
    y => tp2,
    y2 => p9);

  tp5 <= h;

  u7 : g3 port map (
    a => a,
    b => c,
    c => i,
    y => tp4,
    y2 => p13);


  u8 : inv port map (
    a => p20,
    y => e);

  p17 <= e;

  u9 : g2 port map (
    a => j,
    b => e,
    y => h);


  u10 : inv port map (
    a => p15,
    y => f);


  u11 : inv port map (
    a => p19,
    y => t2);

  p11 <= t2;

  u12 : g2 port map (
    a => e,
    b => f,
    y => t3);


  u13 : g2 port map (
    a => t3,
    b => t2,
    y => tp6,
    y2 => j);


  u14 : g2 port map (
    a => f,
    b => j,
    y => i);


  u15 : g2 port map (
    a => p25,
    b => p27,
    y => t4);


  u16 : inv port map (
    a => p26,
    y => t5);


  u17 : g2 port map (
    a => t4,
    b => t5,
    y => p28);


  u18 : g2 port map (
    a => p26,
    b => p24,
    y => t6);

  p21 <= t6;

  u19 : inv port map (
    a => p22,
    y => t7);


  u20 : g2 port map (
    a => t6,
    b => t7,
    y => p23);


  u21 : g2 port map (
    a => p5,
    b => p4,
    y => t8);


  u22 : g2 port map (
    a => t8,
    b => p1,
    y2 => p2);


  u23 : g5 port map (
    a => a,
    b => c,
    c => g,
    d => h,
    e => i,
    y => t9);

  p6 <= t9;
  tp3 <= t9;


end gates;


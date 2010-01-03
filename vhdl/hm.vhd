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
-- HM module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hm is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p12 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p4 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end hm;
architecture gates of hm is
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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p20,
    y => a,
    y2 => b);


  u2 : inv2 port map (
    a => p18,
    y => c,
    y2 => d);


  u3 : inv2 port map (
    a => p16,
    y => e,
    y2 => f);

  p26 <= f;

  u4 : g2 port map (
    a => g,
    b => p8,
    y => t1);


  u5 : g2 port map (
    a => p5,
    b => h,
    y => t2);


  u6 : g2 port map (
    a => t1,
    b => t2,
    y => t3);

  p10 <= t3;
  tp1 <= t3;

  u7 : g2 port map (
    a => g,
    b => p2,
    y => t4);


  u8 : g2 port map (
    a => p6,
    b => h,
    y => t5);


  u9 : g2 port map (
    a => t4,
    b => t5,
    y => t6);

  p4 <= t6;
  tp2 <= t6;

  u10 : inv port map (
    a => p12,
    y => t7);


  u11 : g3 port map (
    a => p7,
    b => g,
    c => t7,
    y => t8);


  u12 : g3 port map (
    a => t7,
    b => h,
    c => p9,
    y => t9);


  u13 : g2 port map (
    a => t8,
    b => t9,
    y => t10);

  p14 <= t10;
  tp3 <= t10;

  u14 : g2 port map (
    a => f,
    b => p27,
    y => p21,
    y2 => t11);

  p19 <= t11;
  tp5 <= t11;

  u15 : g2 port map (
    a => p27,
    b => e,
    y => p23,
    y2 => t12);

  p25 <= t12;
  tp6 <= t12;

  u16 : g2 port map (
    a => b,
    b => d,
    y => t13);

  p13 <= t13;
  tp4 <= t13;

  u17 : g2 port map (
    a => b,
    b => c,
    y => p11);


  u18 : g2 port map (
    a => a,
    b => d,
    y => p22);


  u19 : g2 port map (
    a => a,
    b => c,
    y => p28);


  u20 : g3 port map (
    a => p3,
    b => p1,
    c => p17,
    y => h,
    y2 => g);


  u21 : g2 port map (
    a => p1,
    b => p17,
    y => t14);


  u22 : g2 port map (
    a => t14,
    b => p24,
    y => p15);



end gates;


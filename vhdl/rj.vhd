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
-- RJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rj is
    port (
      p6 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      p4 : out logicsig;
      p5_p3_p2_p1_p15_p13_p16_p14_p24_p26_p27_p28 : out logicsig;
      p7 : out logicsig;
      p8_tp1 : out logicsig;
      p11 : out logicsig;
      p18_tp5 : out logicsig;
      p21_tp6 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig);

end rj;
architecture gates of rj is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

begin -- gates
  u1 : inv2 port map (
    a => p12,
    y => tp2,
    y2 => t1);


  u2 : inv port map (
    a => t1,
    y => p5_p3_p2_p1_p15_p13_p16_p14_p24_p26_p27_p28);


  u3 : inv2 port map (
    a => p9,
    y2 => a);

  tp3 <= a;

  u4 : inv2 port map (
    a => p19,
    y2 => b);

  tp4 <= b;

  u5 : g2 port map (
    a => a,
    b => p23,
    y => c);


  u6 : g2 port map (
    a => a,
    b => p6,
    y => d);


  u7 : g2 port map (
    a => a,
    b => p17,
    y => e);


  u8 : g2 port map (
    a => a,
    b => p20,
    y => f);


  u9 : g2 port map (
    a => a,
    b => p10,
    y => g);


  u10 : inv port map (
    a => p23,
    y => t2);

  p25 <= t2;

  u11 : g2 port map (
    a => t2,
    b => b,
    y => t3);


  u12 : g2 port map (
    a => t3,
    b => c,
    y => p22);


  u13 : inv port map (
    a => p6,
    y => t4);

  p4 <= t4;

  u14 : g2 port map (
    a => t4,
    b => b,
    y => t5);


  u15 : g2 port map (
    a => t5,
    b => d,
    y => p7);


  u16 : inv port map (
    a => p17,
    y => t6);

  p11 <= t6;

  u17 : g2 port map (
    a => t6,
    b => b,
    y => t7);


  u18 : g2 port map (
    a => t7,
    b => e,
    y => p18_tp5);


  u19 : inv port map (
    a => p20,
    y => t8);


  u20 : g2 port map (
    a => t8,
    b => b,
    y => t9);


  u21 : g2 port map (
    a => t9,
    b => f,
    y => p21_tp6);


  u22 : inv port map (
    a => p10,
    y => t10);


  u23 : g2 port map (
    a => t10,
    b => b,
    y => t11);


  u24 : g2 port map (
    a => t11,
    b => g,
    y => p8_tp1);



end gates;


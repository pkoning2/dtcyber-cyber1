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
-- TP module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tp is
    port (
      p1 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2_p4_p6_p8_p10_p12 : out logicsig;
      p3_p5_p7_p9_p11 : out logicsig;
      p16_p18_p20_p22_p24_p26 : out logicsig;
      p17_p19_p21_p23_p25 : out logicsig);

end tp;
architecture gates of tp is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p1,
    y => tp1,
    y2 => t1);


  u2 : inv2 port map (
    a => p14,
    y => tp2,
    y2 => t2);

  tp3 <= t2;

  u3 : g2 port map (
    a => p13,
    b => p15,
    y => tp4,
    y2 => t3);


  u4 : g2 port map (
    a => p27,
    b => p28,
    y => tp5,
    y2 => t4);

  tp6 <= t4;

  u5 : inv port map (
    a => t1,
    y => p3_p5_p7_p9_p11);


  u6 : inv port map (
    a => t2,
    y => p2_p4_p6_p8_p10_p12);


  u7 : inv port map (
    a => t3,
    y => p17_p19_p21_p23_p25);


  u8 : inv port map (
    a => t4,
    y => p16_p18_p20_p22_p24_p26);



end gates;


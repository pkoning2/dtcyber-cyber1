-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- HB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hb is
    port (
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p1_p3_p5_p7_p9 : out logicsig;
      p2_p4_p6_p8_p10 : out logicsig;
      p11_tp2 : out logicsig;
      p12_tp1 : out logicsig;
      p17_tp6 : out logicsig;
      p18_tp5 : out logicsig;
      p19_p21_p23_p25_p27 : out logicsig;
      p20_p22_p24_p26_p28 : out logicsig);

end hb;
architecture gates of hb is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

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
  u1 : g2 port map (
    a => p13,
    b => p14,
    y2 => t1);

  p12_tp1 <= t1;

  u2 : g2 port map (
    a => p14,
    b => p16,
    y2 => t2);

  p11_tp2 <= t2;

  u3 : g2 port map (
    a => p13,
    b => p15,
    y2 => t3);

  p18_tp5 <= t3;

  u4 : g2 port map (
    a => p16,
    b => p15,
    y2 => t4);

  p17_tp6 <= t4;

  u5 : inv2 port map (
    a => t1,
    y2 => p2_p4_p6_p8_p10);


  u6 : inv2 port map (
    a => t2,
    y2 => p1_p3_p5_p7_p9);


  u7 : inv2 port map (
    a => t3,
    y2 => p20_p22_p24_p26_p28);


  u8 : inv2 port map (
    a => t4,
    y2 => p19_p21_p23_p25_p27);



end gates;


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
-- HF module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hf is
    port (
      p1 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp5 : out logicsig;
      p6 : out logicsig;
      p10_p8_p5_p3_p9_p11_p12 : out logicsig;
      p14 : out logicsig;
      p21_p19_p17_p15_p26_p20_p18 : out logicsig;
      p22 : out logicsig;
      p27_p25_p23_p23_p3_p4_p7 : out logicsig);

end hf;
architecture gates of hf is
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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p13,
    y => a);


  u2 : inv2 port map (
    a => p1,
    y => t1,
    y2 => p10_p8_p5_p3_p9_p11_p12);

  tp1 <= t1;

  u3 : g2 port map (
    a => a,
    b => t1,
    y => p14);


  u4 : inv2 port map (
    a => p28,
    y => t2,
    y2 => p27_p25_p23_p23_p3_p4_p7);

  tp3 <= t2;

  u5 : g2 port map (
    a => a,
    b => t2,
    y => p6);


  u6 : inv2 port map (
    a => p16,
    y => t3,
    y2 => p21_p19_p17_p15_p26_p20_p18);

  tp5 <= t3;

  u7 : g2 port map (
    a => a,
    b => t3,
    y => p22);



end gates;


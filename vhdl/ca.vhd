-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- CA module -- monster fanout
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ca is
    port (
      p12 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      tp1_tp3_tp4_tp5_tp6 : out logicsig;
      tp2 : out logicsig;
      p1_p2_p3_p4_p5_p6_p7_p8_p9_p10_p11_p13_p15_p17_p18_p19_p20_p21_p22_p23_p24_p25_p26_p27_p28 : out logicsig);

end ca;
architecture gates of ca is
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

  signal t1 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => p12,
    b => p14,
    c => p16,
    y => tp2,
    y2 => t1);

  tp1_tp3_tp4_tp5_tp6 <= t1;

  u2 : inv port map (
    a => t1,
    y => p1_p2_p3_p4_p5_p6_p7_p8_p9_p10_p11_p13_p15_p17_p18_p19_p20_p21_p22_p23_p24_p25_p26_p27_p28);



end gates;


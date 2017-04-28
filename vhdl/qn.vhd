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
-- QN module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qn is
    port (
      p6 : in  logicsig;
      p8 : in  logicsig;
      p12 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      p1_p2_p3_p4_p5_p7_p9_p11_p13_p14_p15_p16_p17_p18_p19_p21_p23_p25_p27 : out logicsig;
      p10_tp1 : out logicsig;
      p28_tp4 : out logicsig);

end qn;
architecture gates of qn is
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

  signal t1 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p8,
    b => p6,
    y => p10_tp1);


  u2 : g2 port map (
    a => p12,
    b => p26,
    y => tp3,
    y2 => t1);

  tp2 <= t1;

  u3 : inv port map (
    a => t1,
    y => p1_p2_p3_p4_p5_p7_p9_p11_p13_p14_p15_p16_p17_p18_p19_p21_p23_p25_p27);


  u4 : g3 port map (
    a => p24,
    b => p22,
    c => p20,
    y => p28_tp4);



end gates;


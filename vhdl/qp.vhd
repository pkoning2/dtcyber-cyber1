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
-- QP module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qp is
    port (
      p12 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p21 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      p2_p4_p6_p8_p10_p1_p3_p5_p7_p9_p20_p22_p24_p26_p28_p11_p13_p15_p17_p18 : out logicsig;
      p19 : out logicsig;
      p23 : out logicsig);

end qp;
architecture gates of qp is
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

  signal a : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => p12,
    b => p14,
    c => p16,
    y => t1);

  tp3 <= t1;

  u2 : g3 port map (
    a => p16,
    b => p21,
    c => p25,
    y => a);

  tp4 <= a;

  u3 : g2 port map (
    a => p21,
    b => p25,
    y => p19,
    y2 => p23);


  u4 : g2 port map (
    a => t1,
    b => a,
    y => tp1,
    y2 => p2_p4_p6_p8_p10_p1_p3_p5_p7_p9_p20_p22_p24_p26_p28_p11_p13_p15_p17_p18);



end gates;


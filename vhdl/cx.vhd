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
-- CX module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cx is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p3_p6_tp1 : out logicsig;
      p4_p5_tp2 : out logicsig;
      p7_p9_p11_p13 : out logicsig;
      p8_p10_p12_p14 : out logicsig;
      p15_p28_tp6 : out logicsig;
      p16_p27_tp5 : out logicsig;
      p17_p19_p21_p23 : out logicsig;
      p18_p20_p22_p24 : out logicsig);

end cx;
architecture gates of cx is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;


begin -- gates
  u1 : inv2 port map (
    a => p1,
    y => p3_p6_tp1,
    y2 => p8_p10_p12_p14);


  u2 : inv2 port map (
    a => p2,
    y => p4_p5_tp2,
    y2 => p7_p9_p11_p13);


  u3 : inv2 port map (
    a => p25,
    y => p16_p27_tp5,
    y2 => p18_p20_p22_p24);


  u4 : inv2 port map (
    a => p26,
    y => p15_p28_tp6,
    y2 => p17_p19_p21_p23);



end gates;


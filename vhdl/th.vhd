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
-- TH module, rev C -- assorted fanouts
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity th is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p6 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2_p4_p10_p12 : out logicsig;
      p5_p9_p11_p13 : out logicsig;
      p7_p8_p14 : out logicsig;
      p15_p21_p22 : out logicsig;
      p16_p18_p20_p24 : out logicsig;
      p17_p19_p23_p27 : out logicsig);

end th;
architecture gates of th is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;


begin -- gates
  u1 : inv2 port map (
    a => p1,
    y => tp1,
    y2 => p7_p8_p14);


  u2 : inv2 port map (
    a => p6,
    y => tp2,
    y2 => p5_p9_p11_p13);


  u3 : inv2 port map (
    a => p3,
    y => tp3,
    y2 => p2_p4_p10_p12);


  u4 : inv2 port map (
    a => p26,
    y => tp4,
    y2 => p17_p19_p23_p27);


  u5 : inv2 port map (
    a => p25,
    y => tp5,
    y2 => p16_p18_p20_p24);


  u6 : inv2 port map (
    a => p28,
    y => tp6,
    y2 => p15_p21_p22);



end gates;


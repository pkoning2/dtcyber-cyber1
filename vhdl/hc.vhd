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
-- HC module -- 10 way fanout
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hc is
    port (
      p5 : in  logicsig;
      p7 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      tp1_tp2 : out logicsig;
      tp5_tp6 : out logicsig;
      p2_p4_p6_p8_p9_p10_p11_p12_p13_p14 : out logicsig;
      p16_p17_p18_p19_p20_p21_p22_p24_p26_p28 : out logicsig);

end hc;
architecture gates of hc is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;


begin -- gates
  u1 : g2 port map (
    a => p7,
    b => p5,
    y => tp1_tp2,
    y2 => p2_p4_p6_p8_p9_p10_p11_p12_p13_p14);


  u2 : g2 port map (
    a => p23,
    b => p25,
    y => tp5_tp6,
    y2 => p16_p17_p18_p19_p20_p21_p22_p24_p26_p28);



end gates;


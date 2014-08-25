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
-- TD module, rev D -- quad 5-output fanout.  Version with separate outputs
-- for use where some outputs are coax (chassis 12)
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity zd is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p5_p7_p9 : out logicsig;
      p6_p8_p14 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p15_p17_p19_p21_p23 : out logicsig;
      p16_p18_p20_p22_p24 : out logicsig);

end zd;
architecture gates of zd is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;


begin -- gates
  u1 : g2 port map (
    a => p1,
    b => p3,
    y => tp1,
    y2 => p6_p8_p14);

  p10 <= p6_p8_p14;
  p12 <= p6_p8_p14;

  u2 : g2 port map (
    a => p2,
    b => p4,
    y => tp2,
    y2 => p5_p7_p9);

  p11 <= p5_p7_p9;
  p13 <= p5_p7_p9;

  u3 : g2 port map (
    a => p25,
    b => p27,
    y => tp5,
    y2 => p16_p18_p20_p22_p24);


  u4 : g2 port map (
    a => p26,
    b => p28,
    y => tp6,
    y2 => p15_p17_p19_p21_p23);



end gates;


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
-- TC module, rev D -- dual 12 output fanout
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tc is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p2_p4_p6_p7_p8_p9_p10_p11_p12_p13_p15 : out logicsig;
      p17_p18_p19_p20_p21_p22_p23_p24_p25_p26_p27_p28 : out logicsig);

end tc;
architecture gates of tc is
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

  signal a : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p3,
    b => p5,
    y => tp1,
    y2 => a);

  tp2 <= a;

  u2 : inv port map (
    a => a,
    y => p1_p2_p4_p6_p7_p8_p9_p10_p11_p12_p13_p15);


  u4 : g2 port map (
    a => p14,
    b => p16,
    y => tp5,
    y2 => c);

  tp6 <= c;

  u5 : inv port map (
    a => c,
    y => p17_p18_p19_p20_p21_p22_p23_p24_p25_p26_p27_p28);



end gates;


-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- TQ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tq is
    port (
      p2 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p18 : in  logicsig;
      p21 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p3 : out logicsig;
      p4_p6 : out logicsig;
      p8_p10 : out logicsig;
      p9_p11 : out logicsig;
      p14_p16 : out logicsig;
      p15_p17_p19 : out logicsig;
      p20_p22 : out logicsig;
      p23_p25 : out logicsig;
      p26_p28 : out logicsig);

end tq;
architecture gates of tq is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;


begin -- gates
  u1 : inv2 port map (
    a => p12,
    y => tp1,
    y2 => p9_p11);


  u2 : inv2 port map (
    a => p13,
    y => tp2,
    y2 => p14_p16);


  u3 : inv2 port map (
    a => p7,
    y => tp3,
    y2 => p8_p10);


  u4 : inv2 port map (
    a => p24,
    y => tp4,
    y2 => p23_p25);


  u5 : inv2 port map (
    a => p21,
    y => tp6,
    y2 => p20_p22);


  u6 : inv2 port map (
    a => p2,
    y2 => p1_p3);


  u7 : inv2 port map (
    a => p27,
    y2 => p26_p28);


  u8 : inv2 port map (
    a => p5,
    y2 => p4_p6);


  u10 : inv2 port map (
    a => p18,
    y => tp5,
    y2 => p15_p17_p19);



end gates;


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
-- GI module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity gi is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p1 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p10_tp3 : out logicsig;
      p12_tp2 : out logicsig;
      p14_tp1 : out logicsig;
      p15_tp6 : out logicsig;
      p17_tp5 : out logicsig;
      p19_tp4 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end gi;
architecture gates of gi is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;


begin -- gates
  u1 : inv2 port map (
    a => p2,
    y2 => p1);


  u2 : inv2 port map (
    a => p3,
    y2 => p4);


  u3 : inv2 port map (
    a => p5,
    y2 => p6);


  u4 : inv2 port map (
    a => p7,
    y2 => p8);


  u5 : inv2 port map (
    a => p9,
    y2 => p10_tp3);


  u6 : inv2 port map (
    a => p11,
    y2 => p12_tp2);


  u7 : inv2 port map (
    a => p13,
    y2 => p14_tp1);


  u8 : inv2 port map (
    a => p16,
    y2 => p15_tp6);


  u9 : inv2 port map (
    a => p18,
    y2 => p17_tp5);


  u10 : inv2 port map (
    a => p20,
    y2 => p19_tp4);


  u11 : inv2 port map (
    a => p22,
    y2 => p21);


  u12 : inv2 port map (
    a => p24,
    y2 => p23);


  u13 : inv2 port map (
    a => p26,
    y2 => p25);


  u14 : inv2 port map (
    a => p27,
    y2 => p28);



end gates;


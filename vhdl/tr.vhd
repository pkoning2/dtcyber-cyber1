-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- TR module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tr is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p1_tp3 : out logicsig;
      p7_tp2 : out logicsig;
      p10_tp6 : out logicsig;
      p13_tp1 : out logicsig;
      p16_tp5 : out logicsig;
      p19 : out logicsig;
      p22_tp4 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end tr;
architecture gates of tr is
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


begin -- gates
  u1 : g2 port map (
    a => p15,
    b => p17,
    y => p13_tp1);


  u2 : g2 port map (
    a => p3,
    b => p5,
    y => p1_tp3);


  u3 : g2 port map (
    a => p14,
    b => p12,
    y => p16_tp5);


  u4 : g2 port map (
    a => p21,
    b => p23,
    y => p19);


  u5 : g2 port map (
    a => p26,
    b => p24,
    y => p28);


  u6 : g2 port map (
    a => p9,
    b => p11,
    y => p7_tp2);


  u7 : g2 port map (
    a => p20,
    b => p18,
    y => p22_tp4);


  u8 : g3 port map (
    a => p8,
    b => p6,
    c => p4,
    y => p10_tp6);


  u9 : g2 port map (
    a => p27,
    b => p2,
    y => p25);



end gates;


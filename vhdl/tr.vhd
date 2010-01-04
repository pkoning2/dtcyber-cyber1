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
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p7 : out logicsig;
      p10 : out logicsig;
      p13 : out logicsig;
      p16 : out logicsig;
      p19 : out logicsig;
      p22 : out logicsig;
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p15,
    b => p17,
    y => t1);

  p13 <= t1;
  tp1 <= t1;

  u2 : g2 port map (
    a => p3,
    b => p5,
    y => t2);

  p1 <= t2;
  tp3 <= t2;

  u3 : g2 port map (
    a => p14,
    b => p12,
    y => t3);

  p16 <= t3;
  tp5 <= t3;

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
    y => t4);

  p7 <= t4;
  tp2 <= t4;

  u7 : g2 port map (
    a => p20,
    b => p18,
    y => t5);

  p22 <= t5;
  tp4 <= t5;

  u8 : g3 port map (
    a => p8,
    b => p6,
    c => p4,
    y => t6);

  p10 <= t6;
  tp6 <= t6;

  u9 : g2 port map (
    a => p27,
    b => p2,
    y => p25);



end gates;


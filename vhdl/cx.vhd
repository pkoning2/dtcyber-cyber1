-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
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
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end cx;
architecture gates of cx is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p1,
    y => t1,
    y2 => t2);

  p3 <= t1;
  p6 <= t1;
  p8 <= t2;
  p10 <= t2;
  p12 <= t2;
  p14 <= t2;
  tp1 <= t1;

  u2 : inv2 port map (
    a => p2,
    y => t3,
    y2 => t4);

  p4 <= t3;
  p5 <= t3;
  p7 <= t4;
  p9 <= t4;
  p11 <= t4;
  p13 <= t4;
  tp2 <= t3;

  u3 : inv2 port map (
    a => p25,
    y => t5,
    y2 => t6);

  p16 <= t5;
  p18 <= t6;
  p20 <= t6;
  p22 <= t6;
  p24 <= t6;
  p27 <= t5;
  tp5 <= t5;

  u4 : inv2 port map (
    a => p26,
    y => t7,
    y2 => t8);

  p15 <= t7;
  p17 <= t8;
  p19 <= t8;
  p21 <= t8;
  p23 <= t8;
  p28 <= t7;
  tp6 <= t7;


end gates;


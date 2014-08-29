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
-- TH module, rev C -- assorted fanouts -- version with separate outputs
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity zh is
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
      p2 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
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
      p27 : out logicsig);

end zh;
architecture gates of zh is
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

begin -- gates
  u1 : inv2 port map (
    a => p1,
    y => tp1,
    y2 => t1);

  p7 <= t1;
  p8 <= t1;
  p14 <= t1;
  
  u2 : inv2 port map (
    a => p6,
    y => tp2,
    y2 => t2);

  p5 <= t2;
  p9 <= t2;
  p11 <= t2;
  p13 <= t2;

  u3 : inv2 port map (
    a => p3,
    y => tp3,
    y2 => t3);

  p2 <= t3;
  p4 <= t3;
  p10 <= t3;
  p12 <= t3;
  
  u4 : inv2 port map (
    a => p26,
    y => tp4,
    y2 => t4);

  p17 <= t4;
  p19 <= t4;
  p23 <= t4;
  p27 <= t4;
  
  u5 : inv2 port map (
    a => p25,
    y => tp5,
    y2 => t5);

  p16 <= t5;
  p18 <= t5;
  p20 <= t5;
  p24 <= t5;
  
  u6 : inv2 port map (
    a => p28,
    y => tp6,
    y2 => t6);

  p15 <= t6;
  p21 <= t6;
  p22 <= t6;

end gates;


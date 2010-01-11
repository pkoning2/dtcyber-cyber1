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
-- TP module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tp is
    port (
      p1 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
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
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig);

end tp;
architecture gates of tp is
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
    y => tp1,
    y2 => t1);

  u5 : inv port map (
    a => t1,
    y => t5);
  
  p3 <= t5;
  p5 <= t5;
  p7 <= t5;
  p9 <= t5;
  p11 <= t5;

  u2 : inv2 port map (
    a => p14,
    y => tp2,
    y2 => t2);

  tp3 <= t2;

  u6 : inv port map (
    a => t2,
    y => t6);

  p2 <= t6;
  p4 <= t6;
  p6 <= t6;
  p8 <= t6;
  p10 <= t6;
  p12 <= t6;

  u3 : g2 port map (
    a => p13,
    b => p15,
    y => tp4,
    y2 => t3);

  u7 : inv port map (
    a => t3,
    y => t7);
  
  p17 <= t7;
  p19 <= t7;
  p21 <= t7;
  p23 <= t7;
  p25 <= t7;

  u4 : g2 port map (
    a => p27,
    b => p28,
    y => tp5,
    y2 => t4);

  tp6 <= t4;
  
  u8 : inv port map (
    a => t4,
    y => t8);
  
  p16 <= t8;
  p18 <= t8;
  p20 <= t8;
  p22 <= t8;
  p24 <= t8;
  p26 <= t8;


end gates;


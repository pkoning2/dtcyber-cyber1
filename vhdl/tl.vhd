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
-- TL module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tl is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
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
      p20 : out logicsig);

end tl;
architecture gates of tl is
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

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p8,
    y => t1);

  p12 <= t1;
  tp1 <= t1;

  u2 : g3 port map (
    a => p2,
    b => p4,
    c => p6,
    y => t2);

  p10 <= t2;
  tp2 <= t2;

  u3 : g2 port map (
    a => p5,
    b => p7,
    y => t3);

  p11 <= t3;
  tp3 <= t3;

  u4 : g2 port map (
    a => p1,
    b => p3,
    y => t4);

  p9 <= t4;
  tp4 <= t4;

  u5 : inv port map (
    a => p22,
    y => t5);

  p14 <= t5;
  tp5 <= t5;

  u6 : inv port map (
    a => p21,
    y => t6);

  p19 <= t6;
  tp6 <= t6;

  u7 : inv port map (
    a => p23,
    y => p13);


  u8 : inv port map (
    a => p24,
    y => p16);


  u9 : inv port map (
    a => p25,
    y => p15);


  u10 : inv port map (
    a => p26,
    y => p18);


  u11 : inv port map (
    a => p27,
    y => p17);


  u12 : inv port map (
    a => p28,
    y => p20);



end gates;


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
-- CB module -- 6 way fanout and big fanout
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cb is
    port (
      p7 : in  logicsig;
      p8 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end cb;
architecture gates of cb is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p7,
    b => p8,
    y => tp2,
    y2 => t1);

  tp1 <= t1;
  tp3 <= t1;

  u2 : inv port map (
    a => t1,
    y => t2);

  p1 <= t2;
  p2 <= t2;
  p3 <= t2;
  p4 <= t2;
  p5 <= t2;
  p6 <= t2;

  u3 : g2 port map (
    a => p16,
    b => p14,
    y => tp5,
    y2 => t3);

  tp4 <= t3;
  tp6 <= t3;

  u4 : inv port map (
    a => t3,
    y => t4);

  p9 <= t4;
  p10 <= t4;
  p11 <= t4;
  p12 <= t4;
  p13 <= t4;
  p15 <= t4;
  p17 <= t4;
  p18 <= t4;
  p19 <= t4;
  p20 <= t4;
  p21 <= t4;
  p22 <= t4;
  p23 <= t4;
  p24 <= t4;
  p25 <= t4;
  p26 <= t4;
  p27 <= t4;
  p28 <= t4;


end gates;


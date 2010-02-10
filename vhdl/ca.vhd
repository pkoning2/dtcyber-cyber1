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
-- CA module -- monster fanout
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ca is
    port (
      p12 : in  logicsig;
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
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
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

end ca;
architecture gates of ca is
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

begin -- gates
  u1 : g3 port map (
    a => p12,
    b => p14,
    c => p16,
    y => tp2,
    y2 => t1);

  tp1 <= t1;
  tp3 <= t1;
  tp4 <= t1;
  tp5 <= t1;
  tp6 <= t1;

  u2 : inv port map (
    a => t1,
    y => t2);

  p1 <= t2;
  p2 <= t2;
  p3 <= t2;
  p4 <= t2;
  p5 <= t2;
  p6 <= t2;
  p7 <= t2;
  p8 <= t2;
  p9 <= t2;
  p10 <= t2;
  p11 <= t2;
  p13 <= t2;
  p15 <= t2;
  p17 <= t2;
  p18 <= t2;
  p19 <= t2;
  p20 <= t2;
  p21 <= t2;
  p22 <= t2;
  p23 <= t2;
  p24 <= t2;
  p25 <= t2;
  p26 <= t2;
  p27 <= t2;
  p28 <= t2;


end gates;


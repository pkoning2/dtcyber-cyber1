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
-- HC module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hc is
    port (
      p5 : in  logicsig;
      p7 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end hc;
architecture gates of hc is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p7,
    b => p5,
    y => t1,
    y2 => t2);

  p2 <= t2;
  p4 <= t2;
  p6 <= t2;
  p8 <= t2;
  p9 <= t2;
  p10 <= t2;
  p11 <= t2;
  p12 <= t2;
  p13 <= t2;
  p14 <= t2;
  tp1 <= t1;
  tp2 <= t1;

  u2 : g2 port map (
    a => p23,
    b => p25,
    y => t3,
    y2 => t4);

  p16 <= t4;
  p17 <= t4;
  p18 <= t4;
  p19 <= t4;
  p20 <= t4;
  p21 <= t4;
  p22 <= t4;
  p24 <= t4;
  p26 <= t4;
  p28 <= t4;
  tp5 <= t3;
  tp6 <= t3;


end gates;


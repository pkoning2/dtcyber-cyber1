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
-- TD module, rev D -- quad 5-output fanout.  Version with separate outputs
-- for use where some outputs are coax (chassis 12)
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity zd is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p14 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig);

end zd;
architecture gates of zd is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1, t2, t3, t4 : logicsig;
  
begin -- gates
  u1 : g2 port map (
    a => p1,
    b => p3,
    y => tp1,
    y2 => t1);

  p6 <= t1;
  p8 <= t1;
  p14 <= t1;
  p10 <= t1;
  p12 <= t1;

  u2 : g2 port map (
    a => p2,
    b => p4,
    y => tp2,
    y2 => t2);

  p5 <= t2;
  p7 <= t2;
  p9 <= t2;
  p11 <= t2;
  p13 <= t2;

  u3 : g2 port map (
    a => p25,
    b => p27,
    y => tp5,
    y2 => t3);

  p16 <= t3;
  p18 <= t3;
  p20 <= t3;
  p22 <= t3;
  p24 <= t3;

  u4 : g2 port map (
    a => p26,
    b => p28,
    y => tp6,
    y2 => t4);

  p15 <= t4;
  p17 <= t4;
  p19 <= t4;
  p21 <= t4;
  p23 <= t4;

end gates;


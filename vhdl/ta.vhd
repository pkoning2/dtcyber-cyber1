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
-- TA module, rev C -- 6 input gates
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ta is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end ta;
architecture gates of ta is
  component g6
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g6 port map (
    a => p14,
    b => p12,
    c => p10,
    d => p8,
    e => p6,
    f => p4,
    y => t1);

  p2 <= t1;
  tp1 <= t1;

  u2 : g6 port map (
    a => p13,
    b => p11,
    c => p9,
    d => p7,
    e => p5,
    f => p3,
    y => t2);

  p1 <= t2;
  tp2 <= t2;

  u3 : g6 port map (
    a => p16,
    b => p18,
    c => p20,
    d => p22,
    e => p24,
    f => p26,
    y => t3);

  p28 <= t3;
  tp5 <= t3;

  u4 : g6 port map (
    a => p15,
    b => p17,
    c => p19,
    d => p21,
    e => p23,
    f => p25,
    y => t4);

  p27 <= t4;
  tp6 <= t4;


end gates;


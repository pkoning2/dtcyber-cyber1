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
-- TI module, rev C -- 3 and 4 input gates
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ti is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
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
      p11 : out logicsig;
      p13 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig);

end ti;
architecture gates of ti is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
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
  u1 : g4 port map (
    a => p1,
    b => p3,
    c => p5,
    d => p7,
    y => t1);

  p13 <= t1;
  tp1 <= t1;

  u2 : g4 port map (
    a => p2,
    b => p4,
    c => p6,
    d => p8,
    y => t2);

  p9 <= t2;
  tp2 <= t2;

  u3 : g3 port map (
    a => p10,
    b => p12,
    c => p14,
    y => t3);

  p11 <= t3;
  tp3 <= t3;

  u4 : g3 port map (
    a => p15,
    b => p17,
    c => p19,
    y => t4);

  p16 <= t4;
  tp4 <= t4;

  u5 : g4 port map (
    a => p21,
    b => p23,
    c => p25,
    d => p27,
    y => t5);

  p18 <= t5;
  tp5 <= t5;

  u6 : g4 port map (
    a => p22,
    b => p24,
    c => p26,
    d => p28,
    y => t6);

  p20 <= t6;
  tp6 <= t6;


end gates;


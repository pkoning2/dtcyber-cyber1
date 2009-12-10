-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008 by Paul Koning
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
    p1, p3, p5, p7, p2, p4, p6, p8, p10, p12, p14         : in  logicsig;
    p15, p17, p19, p21, p23, p25, p27, p22, p24, p26, p28 : in  logicsig;
    tp1, tp2, tp3, tp4, tp5, tp6                          : out logicsig;  -- test points
    p13, p9, p11, p16, p18, p20                           : out logicsig);

end ti;

architecture gates of ti is
  component g3
    port (
      a, b, c : in  logicsig;                -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component g4
    port (
      a, b, c, d : in  logicsig;             -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  signal t1, t2, t3, t4, t5, t6 : logicsig;
begin  -- gates

  u1 : g4 port map (
    a => p1,
    b => p3,
    c => p5,
    d => p7,
    y => t1);
  tp1 <= t1;
  p13 <= t1;
  u2 : g4 port map (
    a => p2,
    b => p4,
    c => p6,
    d => p8,
    y => t2);
  tp2 <= t2;
  p9 <= t2;
  u3 : g3 port map (
    a => p10,
    b => p12,
    c => p14,
    y => t3);
  tp3 <= t3;
  p11 <= t3;
  u4 : g3 port map (
    a => p15,
    b => p17,
    c => p19,
    y => t4);
  tp4 <= t4;
  p16 <= t4;
  u5 : g4 port map (
    a => p21,
    b => p23,
    c => p25,
    d => p27,
    y => t5);
  tp5 <= t5;
  p18 <= t5;
  u6 : g4 port map (
    a => p22,
    b => p24,
    c => p26,
    d => p28,
    y => t6);
  tp6 <= t6;
  p20 <= t6;

end gates;

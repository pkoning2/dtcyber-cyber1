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
-- IM module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity im is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p12 : out logicsig;
      p16 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end im;
architecture gates of im is
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

  component g5
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p10,
    y => c);

  tp3 <= c;

  u2 : g2 port map (
    a => p13,
    b => p11,
    y => a);

  tp5 <= a;

  u3 : g2 port map (
    a => c,
    b => d,
    y => t1);

  tp6 <= d;

  u4 : g2 port map (
    a => c,
    b => e,
    y => t2);

  tp4 <= e;

  u5 : g2 port map (
    a => c,
    b => f,
    y => t3);

  tp1 <= f;

  u7 : g3 port map (
    a => t1,
    b => t2,
    c => t3,
    y => t4);

  p9 <= t4;
  tp2 <= t4;

  u8 : g5 port map (
    a => p2,
    b => p4,
    c => p6,
    d => p5,
    e => p8,
    y => f);


  u9 : g2 port map (
    a => a,
    b => f,
    y => t5);

  p1 <= t5;
  p3 <= t5;
  p7 <= t5;

  u10 : g5 port map (
    a => p14,
    b => p15,
    c => p17,
    d => p19,
    e => p18,
    y => e);


  u11 : g2 port map (
    a => a,
    b => e,
    y => t6);

  p12 <= t6;
  p16 <= t6;
  p20 <= t6;

  u12 : g5 port map (
    a => p21,
    b => p24,
    c => p23,
    d => p25,
    e => p27,
    y => d);


  u13 : g2 port map (
    a => a,
    b => d,
    y => t7);

  p22 <= t7;
  p26 <= t7;
  p28 <= t7;


end gates;


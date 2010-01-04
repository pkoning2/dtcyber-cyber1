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
-- IS module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mod_is is
    port (
      p4 : in  logicsig;
      p6 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end mod_is;
architecture gates of mod_is is
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

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
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
  signal b : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p6,
    b => p4,
    y => a,
    y2 => t1);

  p2 <= t1;
  tp2 <= a;

  u2 : inv port map (
    a => t1,
    y => t2);

  p1 <= t2;
  p3 <= t2;
  p5 <= t2;
  p7 <= t2;

  u3 : g3 port map (
    a => a,
    b => p18,
    c => p20,
    y => b,
    y2 => t3);

  p16 <= t3;
  tp5 <= b;

  u4 : inv port map (
    a => t3,
    y => t4);

  p15 <= t4;
  p17 <= t4;
  p19 <= t4;
  p21 <= t4;

  u5 : g4 port map (
    a => b,
    b => p23,
    c => a,
    d => p25,
    y => t7,
    y2 => t5);

  tp6 <= t7;
  p27 <= t5;

  u6 : inv port map (
    a => t5,
    y => t6);

  p22 <= t6;
  p24 <= t6;
  p26 <= t6;
  p28 <= t6;

  u7 : g5 port map (
    a => p9,
    b => t7,
    c => a,
    d => p11,
    e => b,
    y => tp1,
    y2 => t8);

  p10 <= t8;

  u8 : inv port map (
    a => t8,
    y => t9);

  p8 <= t9;
  p12 <= t9;
  p13 <= t9;
  p14 <= t9;


end gates;


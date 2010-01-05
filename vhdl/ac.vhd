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
-- AC module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ac is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end ac;
architecture gates of ac is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;

begin -- gates
  u1 : g5 port map (
    a => p2,
    b => p4,
    c => p6,
    d => p8,
    e => a,
    y2 => t1);

  p5 <= a;
  p7 <= a;

  u2 : g3 port map (
    a => p3,
    b => p1,
    c => t1,
    y => t2);


  u3 : inv2 port map (
    a => p10,
    y => t3,
    y2 => a);


  u4 : g3 port map (
    a => p27,
    b => p9,
    c => t3,
    y => t4);

  tp1 <= t4;

  u5 : rs2flop port map (
    r => t4,
    s => t2,
    s2 => p16,
    q => t5);


  u6 : inv2 port map (
    a => t5,
    y => p12,
    y2 => t6);

  p11 <= t6;
  tp4 <= t6;

  u7 : g2 port map (
    a => p20,
    b => p22,
    y2 => t7);

  p19 <= t7;
  p21 <= t7;
  p23 <= t7;
  tp2 <= t7;

  u8 : inv2 port map (
    a => p25,
    y2 => t8);

  p24 <= t8;
  p26 <= t8;
  p28 <= t8;
  tp3 <= t8;

  u9 : inv port map (
    a => p14,
    y => t9);

  p18 <= t9;
  tp5 <= t9;

  u10 : g2 port map (
    a => p13,
    b => p17,
    y => t10);

  p15 <= t10;
  tp6 <= t10;


end gates;


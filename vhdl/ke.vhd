-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- KE module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ke is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end ke;
architecture gates of ke is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p3,
    b => p6,
    y => t1);


  u2 : inv port map (
    a => p2,
    y => t2);


  u3 : g2 port map (
    a => p1,
    b => t2,
    y => t3);


  u4 : g2 port map (
    a => p2,
    b => p4,
    y => t4);


  u5 : g6 port map (
    a => p8,
    b => p10,
    c => t1,
    d => p5,
    e => t3,
    f => t4,
    y => t5,
    y2 => p9);

  p7 <= t5;
  tp1 <= t5;

  u6 : g4 port map (
    a => p27,
    b => p25,
    c => p23,
    d => p21,
    y => t6,
    y2 => p28);

  p26 <= t6;
  tp2 <= t6;

  u7 : g4 port map (
    a => p22,
    b => p18,
    c => p19,
    d => p20,
    y => t7,
    y2 => p16);

  p24 <= t7;
  tp5 <= t7;

  u8 : g4 port map (
    a => p17,
    b => p15,
    c => p11,
    d => p13,
    y => t8,
    y2 => p12);

  p14 <= t8;
  tp6 <= t8;


end gates;


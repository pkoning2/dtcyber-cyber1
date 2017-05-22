-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- NI module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ni is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2_tp3 : out logicsig;
      p8 : out logicsig;
      p12_tp1 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p19_tp4 : out logicsig;
      p21 : out logicsig;
      p24 : out logicsig;
      p28 : out logicsig);

end ni;
architecture gates of ni is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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
  signal c : logicsig;
  signal d : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p11,
    b => p9,
    y => t1);


  u2 : inv port map (
    a => p7,
    y => t2);


  u3 : g2 port map (
    a => t1,
    b => t2,
    y => c);

  p12_tp1 <= c;

  u4 : g2 port map (
    a => p6,
    b => p10,
    y => t3);


  u5 : inv port map (
    a => p4,
    y => t4);


  u6 : g2 port map (
    a => t3,
    b => t4,
    y => b,
    y2 => p13);

  tp2 <= b;

  u7 : g2 port map (
    a => p3,
    b => p5,
    y => t5);


  u8 : inv port map (
    a => p1,
    y => t6);


  u9 : g2 port map (
    a => t5,
    b => t6,
    y => a,
    y2 => p8);

  p2_tp3 <= a;

  u10 : g2 port map (
    a => p16,
    b => p18,
    y => t7);


  u11 : g2 port map (
    a => a,
    b => f,
    y => t8);

  p14 <= t8;
  tp6 <= f;

  u12 : g2 port map (
    a => t7,
    b => t8,
    y => p19_tp4);


  u13 : g2 port map (
    a => p25,
    b => p27,
    y2 => d);

  tp5 <= d;

  u14 : g2 port map (
    a => c,
    b => d,
    y => p28);


  u15 : g2 port map (
    a => p15,
    b => p17,
    y => t10);


  u16 : g2 port map (
    a => b,
    b => d,
    y => t11);

  p21 <= t11;

  u17 : g2 port map (
    a => t10,
    b => t11,
    y => f);


  u18 : g2 port map (
    a => f,
    b => b,
    y => p24);



end gates;


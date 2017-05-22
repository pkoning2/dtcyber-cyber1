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
-- NS module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ns is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp4 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p9_tp2 : out logicsig;
      p12 : out logicsig;
      p15 : out logicsig;
      p16_tp6 : out logicsig;
      p19 : out logicsig;
      p21_tp3 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p26_tp5 : out logicsig;
      p27 : out logicsig);

end ns;
architecture gates of ns is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p2,
    y2 => p1);


  u2 : inv port map (
    a => p4,
    y => t1);


  u3 : g2 port map (
    a => p2,
    b => t1,
    y2 => t2);

  p3 <= t2;

  u4 : inv port map (
    a => p6,
    y => t3);


  u5 : g2 port map (
    a => t2,
    b => t3,
    y => tp1,
    y2 => t4);

  p5 <= t4;

  u6 : g2 port map (
    a => p8,
    b => t4,
    y => tp4,
    y2 => p7);


  u7 : g2 port map (
    a => p18,
    b => p17,
    y => p15);


  u8 : inv port map (
    a => p20,
    y => t6);


  u9 : g2 port map (
    a => t6,
    b => p17,
    y => p19);


  u10 : g2 port map (
    a => p20,
    b => p14,
    y => p16_tp6);


  u11 : g2 port map (
    a => p14,
    b => p10,
    y => p12);


  u12 : inv port map (
    a => p11,
    y => t7);


  u13 : g2 port map (
    a => t7,
    b => p13,
    y => p9_tp2);


  u14 : inv port map (
    a => p23,
    y => t8);


  u15 : inv port map (
    a => p24,
    y => t9);


  u16 : g2 port map (
    a => t8,
    b => p24,
    y => p22);


  u17 : g2 port map (
    a => p23,
    b => t9,
    y => p21_tp3);


  u18 : inv port map (
    a => p25,
    y => t10);


  u19 : inv port map (
    a => p28,
    y => t11);


  u20 : g2 port map (
    a => t10,
    b => p28,
    y => p26_tp5);


  u22 : g2 port map (
    a => t11,
    b => p25,
    y => p27);



end gates;


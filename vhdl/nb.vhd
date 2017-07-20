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
-- NB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nb is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p10_p12_p11_p13_p1_p3_p5_p7_p28_p26_p24_p22_p25_p23_p21_p19 : out logicsig;
      p15_tp2 : out logicsig);

end nb;
architecture gates of nb is
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
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p6,
    y => t1);


  u2 : inv port map (
    a => p18,
    y => t2);


  u3 : inv port map (
    a => p8,
    y => t3);


  u4 : g2 port map (
    a => p20,
    b => t3,
    y => t4);

  tp5 <= t4;

  u5 : g2 port map (
    a => t4,
    b => p16,
    y => t5);

  tp6 <= t5;

  u6 : g3 port map (
    a => p18,
    b => t4,
    c => p16,
    y => t8);

  tp3 <= t8;

  u7 : g2 port map (
    a => t2,
    b => t5,
    y => t7);


  u8 : g2 port map (
    a => t7,
    b => t8,
    y => t9,
    y2 => t10);

  tp1 <= t9;
  tp4 <= t10;

  u9 : inv port map (
    a => t10,
    y => p10_p12_p11_p13_p1_p3_p5_p7_p28_p26_p24_p22_p25_p23_p21_p19);


  u11 : g3 port map (
    a => t10,
    b => p27,
    c => t1,
    y => t11);


  u12 : g3 port map (
    a => p9,
    b => p2,
    c => t9,
    y => t12);


  u13 : g4 port map (
    a => p4,
    b => t11,
    c => t12,
    d => p14,
    y => p15_tp2);



end gates;


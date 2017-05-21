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
-- NV module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nv is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp3 : out logicsig;
      p5_tp4 : out logicsig;
      p6_p10_p16_tp1 : out logicsig;
      p15 : out logicsig;
      p17_tp2 : out logicsig;
      p18_p1_p4 : out logicsig;
      p19_tp5 : out logicsig;
      p22 : out logicsig;
      p28_tp6 : out logicsig);

end nv;
architecture gates of nv is
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

  signal k : logicsig;
  signal q : logicsig;
  signal r : logicsig;
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
  u1 : inv port map (
    a => p9,
    y => r);


  u2 : g3 port map (
    a => p8,
    b => p21,
    c => q,
    y => k);


  u3 : g2 port map (
    a => r,
    b => k,
    y => p6_p10_p16_tp1);


  u4 : inv port map (
    a => k,
    y => t1);

  tp3 <= t1;

  u5 : g2 port map (
    a => t1,
    b => r,
    y => p18_p1_p4);


  u6 : inv port map (
    a => p20,
    y => t2);


  u7 : g2 port map (
    a => t2,
    b => p13,
    y => p19_tp5);


  u8 : inv2 port map (
    a => p11,
    y => t3,
    y2 => t4);


  u9 : g2 port map (
    a => t4,
    b => p12,
    y => p17_tp2);


  u10 : g2 port map (
    a => t3,
    b => p14,
    y => p15);


  u11 : inv port map (
    a => p2,
    y => t5);


  u12 : inv port map (
    a => p3,
    y => t6);


  u13 : g2 port map (
    a => t5,
    b => t6,
    y2 => q);


  u14 : g3 port map (
    a => q,
    b => k,
    c => r,
    y => p5_tp4);


  u15 : inv port map (
    a => p26,
    y => t7);


  u16 : inv port map (
    a => p27,
    y => t8);


  u17 : g2 port map (
    a => t7,
    b => p27,
    y => t9);


  u18 : g2 port map (
    a => t8,
    b => p26,
    y => t10);


  u19 : g3 port map (
    a => p23,
    b => t9,
    c => t10,
    y => p28_tp6);


  u20 : g3 port map (
    a => p25,
    b => p24,
    c => p23,
    y => p22);



end gates;


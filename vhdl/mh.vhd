-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- MH module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mh is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p17 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      p5_p6_tp1 : out logicsig;
      p7_p9_p14_tp2 : out logicsig;
      p12_p13_tp3 : out logicsig;
      p15_p19_p20_tp5 : out logicsig;
      p16_p18_tp4 : out logicsig;
      p22_p24_tp6 : out logicsig);

end mh;
architecture gates of mh is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal t3 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => b,
    b => p10,
    y => p5_p6_tp1);


  u2 : g3 port map (
    a => d,
    b => e,
    c => f,
    y => p7_p9_p14_tp2);


  u3 : inv port map (
    a => p8,
    y => d);


  u4 : inv port map (
    a => p1,
    y => a);


  u5 : g2 port map (
    a => p1,
    b => p3,
    y => e);


  u6 : g3 port map (
    a => p3,
    b => p2,
    c => p4,
    y => f);


  u7 : inv2 port map (
    a => p2,
    y => b,
    y2 => t3);


  u8 : g2 port map (
    a => t3,
    b => p4,
    y => c);


  u9 : g3 port map (
    a => a,
    b => c,
    c => p11,
    y => p12_p13_tp3);


  u10 : g2 port map (
    a => h,
    b => p21,
    y => p22_p24_tp6);


  u11 : g3 port map (
    a => j,
    b => k,
    c => l,
    y => p15_p19_p20_tp5);


  u12 : inv port map (
    a => p23,
    y => j);


  u13 : inv port map (
    a => p28,
    y => g);


  u14 : g2 port map (
    a => p28,
    b => p26,
    y => k);


  u15 : g3 port map (
    a => p26,
    b => p27,
    c => p25,
    y => l);


  u16 : inv2 port map (
    a => p27,
    y => h,
    y2 => t7);


  u17 : g2 port map (
    a => t7,
    b => p25,
    y => i);


  u18 : g3 port map (
    a => g,
    b => i,
    c => p17,
    y => p16_p18_tp4);



end gates;


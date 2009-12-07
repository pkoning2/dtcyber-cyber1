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
-- MH module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
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
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig);

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
    a => b,
    b => p10,
    y => t1);

  p5 <= t1;
  p6 <= t1;
  tp1 <= t1;

  u2 : g3 port map (
    a => d,
    b => e,
    c => f,
    y => t2);

  p7 <= t2;
  p9 <= t2;
  p14 <= t2;
  tp2 <= t2;

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
    y => t4);

  p12 <= t4;
  p13 <= t4;
  tp3 <= t4;

  u10 : g2 port map (
    a => h,
    b => p21,
    y => t5);

  p22 <= t5;
  p24 <= t5;
  tp6 <= t5;

  u11 : g3 port map (
    a => j,
    b => k,
    c => l,
    y => t6);

  p15 <= t6;
  p19 <= t6;
  p20 <= t6;
  tp5 <= t6;

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
    y => t8);

  p16 <= t8;
  p18 <= t8;
  tp4 <= t8;


end gates;


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
-- MI module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity mi is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
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
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end mi;
architecture gates of mi is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : g6 port map (
    a => g,
    b => h,
    c => i,
    d => p6,
    e => p4,
    f => p2,
    y => t1,
    y2 => p9);

  p3 <= t1;
  p10 <= h;
  tp1 <= t1;

  u2 : g3 port map (
    a => g,
    b => h,
    c => i,
    y => t2);

  p1 <= t2;
  tp2 <= t2;

  u3 : g6 port map (
    a => d,
    b => e,
    c => f,
    d => g,
    e => h,
    f => i,
    y => t3,
    y2 => p11);

  p14 <= t3;
  p16 <= e;
  tp3 <= t3;

  u4 : g3 port map (
    a => a,
    b => b,
    c => c,
    y => t4);

  p15 <= t4;
  p20 <= c;
  p24 <= b;
  p26 <= b;
  tp4 <= t4;

  u5 : g3 port map (
    a => d,
    b => e,
    c => f,
    y => t5);

  p13 <= t5;
  tp5 <= t5;

  u6 : g6 port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    f => f,
    y => t6,
    y2 => p28);

  p22 <= t6;
  tp6 <= t6;

  u7 : inv port map (
    a => p7,
    y => g);


  u8 : inv port map (
    a => p8,
    y => h);


  u9 : inv port map (
    a => p5,
    y => i);


  u10 : inv port map (
    a => p23,
    y => d);


  u11 : inv port map (
    a => p17,
    y => e);


  u12 : inv port map (
    a => p19,
    y => f);


  u13 : inv port map (
    a => p21,
    y => a);

  p18 <= a;

  u14 : inv port map (
    a => p27,
    y => b);


  u15 : inv port map (
    a => p25,
    y => c);



end gates;


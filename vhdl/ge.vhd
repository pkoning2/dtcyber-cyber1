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
-- GE module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ge is
    port (
      p1 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p6 : out logicsig;
      p10 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig);

end ge;
architecture gates of ge is
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

  signal a : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p9,
    y => p15);


  u2 : g2 port map (
    a => p9,
    b => p13,
    y => c);

  tp2 <= c;

  u3 : inv port map (
    a => p11,
    y => d);

  tp1 <= d;

  u4 : inv port map (
    a => p18,
    y => a);

  p27 <= a;

  u5 : g2 port map (
    a => p17,
    b => a,
    y => e);

  tp5 <= e;

  u6 : inv port map (
    a => p20,
    y => t1);


  u7 : g3 port map (
    a => a,
    b => p19,
    c => t1,
    y => f);

  tp4 <= f;

  u8 : g3 port map (
    a => p21,
    b => a,
    c => p23,
    y => g);

  tp6 <= g;

  u9 : g5 port map (
    a => c,
    b => d,
    c => e,
    d => f,
    e => g,
    y => t2,
    y2 => t4);

  tp3 <= t2;
  p16 <= t2;
  p10 <= t4;
  p14 <= t4;
  
  u10 : g2 port map (
    a => t2,
    b => p22,
    y => p26);


  u11 : g2 port map (
    a => t2,
    b => h,
    y => p6);


  u12 : g3 port map (
    a => p5,
    b => p1,
    c => p7,
    y2 => t3);


  u13 : g2 port map (
    a => t2,
    b => t3,
    y => p24);


  u14 : g2 port map (
    a => p1,
    b => p7,
    y2 => h);



end gates;


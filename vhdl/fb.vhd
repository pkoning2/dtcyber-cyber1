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
-- FB module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity fb is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p17 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end fb;
architecture gates of fb is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal m : logicsig;
  signal n : logicsig;
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
  u1 : g4 port map (
    a => e,
    b => f,
    c => g,
    d => h,
    y => tp1,
    y2 => t1);

  tp3 <= h;

  u2 : inv port map (
    a => t1,
    y => t2);

  p7 <= t2;
  p8 <= t2;
  p9 <= t2;
  p10 <= t2;
  p12 <= t2;

  u3 : inv port map (
    a => p4,
    y => a);


  u4 : inv port map (
    a => p3,
    y => b);


  u5 : inv port map (
    a => p1,
    y => c);


  u6 : inv port map (
    a => p2,
    y => t3);


  u7 : g4 port map (
    a => a,
    b => b,
    c => c,
    d => t3,
    y => t4);

  p6 <= t4;
  tp2 <= t4;

  u8 : inv port map (
    a => p5,
    y => e);


  u9 : inv port map (
    a => p15,
    y => i);


  u10 : g2 port map (
    a => p15,
    b => a,
    y => f);


  u11 : inv2 port map (
    a => p16,
    y => j,
    y2 => t5);


  u12 : g2 port map (
    a => b,
    b => t5,
    y => l);

  tp4 <= l;

  u13 : g3 port map (
    a => p16,
    b => a,
    c => b,
    y => g);


  u14 : inv2 port map (
    a => p13,
    y => k,
    y2 => t6);


  u15 : g2 port map (
    a => c,
    b => t6,
    y => m);

  tp5 <= m;

  u16 : g3 port map (
    a => t6,
    b => c,
    c => b,
    y => n);

  tp6 <= n;

  u17 : g4 port map (
    a => a,
    b => p13,
    c => c,
    d => b,
    y => h);


  u18 : inv port map (
    a => p27,
    y => t7);

  p26 <= t7;
  p28 <= t7;

  u19 : g2 port map (
    a => p25,
    b => k,
    y => t8);

  p23 <= t8;
  p24 <= t8;

  u20 : g3 port map (
    a => p19,
    b => j,
    c => m,
    y => t9);

  p21 <= t9;
  p22 <= t9;

  u21 : g4 port map (
    a => p18,
    b => i,
    c => l,
    d => n,
    y => t10);

  p17 <= t10;
  p20 <= t10;


end gates;


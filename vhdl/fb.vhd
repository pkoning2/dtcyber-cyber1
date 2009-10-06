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
      p1 : in  std_logic;
      p2 : in  std_logic;
      p3 : in  std_logic;
      p4 : in  std_logic;
      p5 : in  std_logic;
      p13 : in  std_logic;
      p15 : in  std_logic;
      p16 : in  std_logic;
      p18 : in  std_logic;
      p19 : in  std_logic;
      p25 : in  std_logic;
      p27 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p6 : out std_logic;
      p7 : out std_logic;
      p8 : out std_logic;
      p9 : out std_logic;
      p10 : out std_logic;
      p12 : out std_logic;
      p17 : out std_logic;
      p20 : out std_logic;
      p21 : out std_logic;
      p22 : out std_logic;
      p23 : out std_logic;
      p24 : out std_logic;
      p26 : out std_logic;
      p28 : out std_logic);

end fb;
architecture gates of fb is
  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component g3
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component g4
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component inv
    port (
      a : in  std_logic;
      y : out std_logic);

  end component;

  component inv2
    port (
      a : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal e : std_logic;
  signal f : std_logic;
  signal g : std_logic;
  signal h : std_logic;
  signal i : std_logic;
  signal j : std_logic;
  signal k : std_logic;
  signal l : std_logic;
  signal m : std_logic;
  signal n : std_logic;
  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;
  signal t7 : std_logic;
  signal t8 : std_logic;
  signal t9 : std_logic;
  signal t10 : std_logic;

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


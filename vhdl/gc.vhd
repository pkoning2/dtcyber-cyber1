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
-- GC module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity gc is
    port (
      p2 : in  std_logic;
      p3 : in  std_logic;
      p4 : in  std_logic;
      p5 : in  std_logic;
      p6 : in  std_logic;
      p10 : in  std_logic;
      p12 : in  std_logic;
      p16 : in  std_logic;
      p18 : in  std_logic;
      p20 : in  std_logic;
      p22 : in  std_logic;
      p23 : in  std_logic;
      p24 : in  std_logic;
      p25 : in  std_logic;
      p26 : in  std_logic;
      p27 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p7 : out std_logic;
      p8 : out std_logic;
      p9 : out std_logic;
      p11 : out std_logic;
      p13 : out std_logic;
      p14 : out std_logic;
      p15 : out std_logic;
      p17 : out std_logic;
      p19 : out std_logic;
      p21 : out std_logic;
      p28 : out std_logic);

end gc;
architecture gates of gc is
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

  component rsflop
    port (
      r : in  std_logic;
      r2 : in  std_logic := '1';
      r3 : in  std_logic := '1';
      r4 : in  std_logic := '1';
      s : in  std_logic;
      s2 : in  std_logic := '1';
      s3 : in  std_logic := '1';
      s4 : in  std_logic := '1';
      q : out std_logic;
      qb : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal d : std_logic;
  signal e : std_logic;
  signal f : std_logic;
  signal g : std_logic;
  signal h : std_logic;
  signal i : std_logic;
  signal j : std_logic;
  signal k : std_logic;
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
  signal t11 : std_logic;
  signal t12 : std_logic;

begin -- gates
  u1 : rsflop port map (
    r => a,
    s => p2,
    s2 => p3,
    s3 => p4,
    s4 => p5,
    q => t1);

  tp1 <= t1;

  u2 : inv port map (
    a => t1,
    y => j);

  p1 <= j;

  u3 : inv2 port map (
    a => p6,
    y => t2,
    y2 => k);


  u4 : g2 port map (
    a => j,
    b => k,
    y => g);

  p8 <= g;

  u5 : g2 port map (
    a => t1,
    b => t2,
    y => f);

  p7 <= f;

  u6 : g2 port map (
    a => f,
    b => g,
    y => p9);


  u7 : rsflop port map (
    r => a,
    s => p22,
    s2 => p20,
    s3 => p18,
    s4 => p16,
    q => t3);

  tp5 <= t3;

  u8 : inv2 port map (
    a => p10,
    y => t4,
    y2 => i);


  u9 : inv port map (
    a => t3,
    y => h);

  p15 <= h;

  u10 : g2 port map (
    a => h,
    b => i,
    y => e,
    y2 => p17);


  u11 : g2 port map (
    a => t3,
    b => t4,
    y => d);


  u12 : g2 port map (
    a => d,
    b => e,
    y => p11);


  u13 : rsflop port map (
    r => a,
    s => p27,
    s2 => p26,
    s3 => p23,
    s4 => p24,
    q => t5);

  tp6 <= t5;

  u14 : inv port map (
    a => p12,
    y => a);


  u15 : inv2 port map (
    a => p25,
    y => t6,
    y2 => t7);


  u16 : inv port map (
    a => t5,
    y => t8);

  p28 <= t8;

  u17 : g2 port map (
    a => t8,
    b => t7,
    y => c);


  u18 : g2 port map (
    a => t5,
    b => t6,
    y => b);


  u19 : g2 port map (
    a => b,
    b => c,
    y => p14);


  u20 : g3 port map (
    a => b,
    b => d,
    c => f,
    y => t9,
    y2 => p19);

  p13 <= t9;
  tp4 <= t9;

  u21 : g4 port map (
    a => b,
    b => d,
    c => k,
    d => j,
    y => t10);

  tp2 <= t10;

  u22 : g3 port map (
    a => h,
    b => i,
    c => b,
    y => t11);


  u23 : g3 port map (
    a => t10,
    b => c,
    c => t11,
    y => t12);

  p21 <= t12;
  tp3 <= t12;


end gates;


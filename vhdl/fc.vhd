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
-- FC module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity fc is
    port (
      p1 : in  std_logic;
      p3 : in  std_logic;
      p7 : in  std_logic;
      p9 : in  std_logic;
      p13 : in  std_logic;
      p14 : in  std_logic;
      p17 : in  std_logic;
      p19 : in  std_logic;
      p20 : in  std_logic;
      p21 : in  std_logic;
      p22 : in  std_logic;
      p23 : in  std_logic;
      p25 : in  std_logic;
      p26 : in  std_logic;
      tp1 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p2 : out std_logic;
      p4 : out std_logic;
      p5 : out std_logic;
      p6 : out std_logic;
      p8 : out std_logic;
      p10 : out std_logic;
      p11 : out std_logic;
      p12 : out std_logic;
      p15 : out std_logic;
      p16 : out std_logic;
      p18 : out std_logic;
      p24 : out std_logic;
      p28 : out std_logic);

end fc;
architecture gates of fc is
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

  component g5
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      e : in  std_logic;
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
  signal d : std_logic;
  signal e : std_logic;
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

begin -- gates
  u1 : inv port map (
    a => p1,
    y => a);

  p2 <= a;

  u2 : inv port map (
    a => p3,
    y => b);

  p4 <= b;

  u3 : inv port map (
    a => p7,
    y => c);

  p8 <= c;

  u4 : inv port map (
    a => p9,
    y => d);

  p10 <= d;

  u5 : inv2 port map (
    a => p13,
    y => e,
    y2 => p11);


  u6 : g2 port map (
    a => a,
    b => b,
    y => p6,
    y2 => g);


  u7 : g2 port map (
    a => b,
    b => c,
    y => p5,
    y2 => h);


  u8 : g2 port map (
    a => c,
    b => d,
    y => t1);

  p12 <= t1;
  tp1 <= t1;

  u9 : g2 port map (
    a => d,
    b => e,
    y => p15);


  u10 : g2 port map (
    a => p26,
    b => p23,
    y2 => j);


  u11 : g3 port map (
    a => p26,
    b => p23,
    c => p25,
    y2 => k);


  u12 : g2 port map (
    a => e,
    b => a,
    y2 => i);

  p11 <= i;
  tp3 <= i;

  u13 : g3 port map (
    a => i,
    b => h,
    c => p21,
    y => t2);

  tp5 <= t2;

  u14 : g2 port map (
    a => p22,
    b => i,
    y => t3);

  tp6 <= t3;

  u15 : inv port map (
    a => p14,
    y => t4);


  u16 : g2 port map (
    a => p19,
    b => e,
    y => t5);


  u17 : g3 port map (
    a => p20,
    b => g,
    c => e,
    y => t6);


  u18 : g5 port map (
    a => t2,
    b => t3,
    c => t4,
    d => t5,
    e => t6,
    y => t7,
    y2 => p16);

  tp4 <= t7;

  u19 : g2 port map (
    a => p17,
    b => t7,
    y => p18);


  u20 : g2 port map (
    a => t7,
    b => j,
    y => p24);


  u21 : g2 port map (
    a => t7,
    b => k,
    y => p28);



end gates;


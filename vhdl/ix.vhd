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
-- IX module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ix is
    port (
      p10 : in std_logic;
      p12 : in std_logic;
      p24 : in std_logic;
      p25 : in std_logic;
      p26 : in std_logic;
      p28 : in std_logic;
      p8 : in std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p17 : out std_logic;
      p19 : out std_logic;
      p3 : out std_logic;
      p5 : out std_logic;
      p6 : out std_logic);

end ix;
architecture gates of ix is
  component g2
    port (
      a : in std_logic;
      b : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component g3
    port (
      a : in std_logic;
      b : in std_logic;
      c : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component g4
    port (
      a : in std_logic;
      b : in std_logic;
      c : in std_logic;
      d : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component inv
    port (
      a : in std_logic;
      y : out std_logic);

  end component;

  component inv2
    port (
      a : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal d : std_logic;
  signal e : std_logic;
  signal f : std_logic;
  signal k : std_logic;
  signal t1 : std_logic;
  signal t10 : std_logic;
  signal t11 : std_logic;
  signal t12 : std_logic;
  signal t13 : std_logic;
  signal t14 : std_logic;
  signal t15 : std_logic;
  signal t2 : std_logic;
  signal t20 : std_logic;
  signal t21 : std_logic;
  signal t22 : std_logic;
  signal t23 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;
  signal t7 : std_logic;
  signal t8 : std_logic;
  signal t9 : std_logic;

begin -- gates
  u1 : inv2 port map (
    a => k,
    y => t1,
    y2 => t2);

  u10 : g2 port map (
    a => c,
    b => t12,
    y => t13);

  u11 : g2 port map (
    a => d,
    b => t11,
    y => t14);

  u12 : g2 port map (
    a => t13,
    b => t14,
    y => t15);

  u13 : inv port map (
    a => p8,
    y => t20);

  u14 : inv2 port map (
    a => t20,
    y => e,
    y2 => f);

  u15 : inv port map (
    a => p12,
    y => t21);

  u16 : inv2 port map (
    a => t21,
    y => a,
    y2 => b);

  u17 : inv port map (
    a => p10,
    y => t22);

  u18 : inv2 port map (
    a => t22,
    y => c,
    y2 => d);

  u19 : g3 port map (
    a => a,
    b => c,
    c => e,
    y2 => t23);

  u2 : g2 port map (
    a => a,
    b => t2,
    y => t3);

  u20 : g4 port map (
    a => p26,
    b => p24,
    c => p28,
    d => p25,
    y2 => k);

  u3 : g2 port map (
    a => t1,
    b => b,
    y => t4);

  u4 : g2 port map (
    a => t3,
    b => t4,
    y => t5);

  u5 : g3 port map (
    a => a,
    b => c,
    c => k,
    y => t6,
    y2 => t7);

  u6 : g2 port map (
    a => e,
    b => t7,
    y => t8);

  u7 : g2 port map (
    a => f,
    b => t6,
    y => t9);

  u8 : g2 port map (
    a => t8,
    b => t9,
    y => t10);

  u9 : g2 port map (
    a => a,
    b => k,
    y => t11,
    y2 => t12);

p1 <= t23;
p17 <= t10;
p19 <= t15;
p3 <= t23;
p5 <= t23;
p6 <= t5;
tp1 <= t5;
tp2 <= e;
tp3 <= a;
tp4 <= c;
tp5 <= t10;
tp6 <= t15;

end gates;


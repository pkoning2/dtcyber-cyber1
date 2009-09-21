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
-- KD module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity kd is
    port (
      p25 : in std_logic;
      p28 : in std_logic;
      p3 : in std_logic;
      p6 : in std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p23 : out std_logic;
      p8 : out std_logic);

end kd;
architecture gates of kd is
  component g2
    port (
      a : in std_logic;
      b : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component g6
    port (
      a : in std_logic;
      b : in std_logic;
      c : in std_logic;
      d : in std_logic;
      e : in std_logic;
      f : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component inv
    port (
      a : in std_logic;
      y : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal d : std_logic;
  signal e : std_logic;
  signal g : std_logic;
  signal i : std_logic;
  signal j : std_logic;
  signal k : std_logic;
  signal l : std_logic;
  signal m : std_logic;
  signal n : std_logic;
  signal o : std_logic;
  signal p : std_logic;
  signal q : std_logic;
  signal r : std_logic;
  signal s : std_logic;
  signal t : std_logic;
  signal t1 : std_logic;
  signal t10 : std_logic;
  signal t11 : std_logic;
  signal t12 : std_logic;
  signal t13 : std_logic;
  signal t14 : std_logic;
  signal t15 : std_logic;
  signal t16 : std_logic;
  signal t17 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;
  signal t7 : std_logic;
  signal t8 : std_logic;
  signal t9 : std_logic;

begin -- gates
  u1 : g2 port map (
    a => c,
    b => p3,
    y => t1);

  u10 : inv port map (
    a => p28,
    y => a);

  u11 : g2 port map (
    a => t5,
    b => t6,
    y => e,
    y2 => t7);

  u12 : g2 port map (
    a => g,
    b => t7,
    y => t8);

  u13 : g2 port map (
    a => t4,
    b => t8,
    y => t9);

  u14 : g2 port map (
    a => d,
    b => c,
    y => s,
    y2 => t);

  u15 : g2 port map (
    a => d,
    b => b,
    y => q,
    y2 => r);

  u16 : g2 port map (
    a => c,
    b => b,
    y => o,
    y2 => p);

  u17 : g2 port map (
    a => d,
    b => a,
    y => m,
    y2 => n);

  u18 : g2 port map (
    a => c,
    b => a,
    y => k,
    y2 => l);

  u19 : g2 port map (
    a => b,
    b => a,
    y => i,
    y2 => j);

  u2 : inv port map (
    a => p3,
    y => d);

  u20 : g2 port map (
    a => t,
    b => j,
    y2 => t10);

  u21 : g2 port map (
    a => j,
    b => s,
    y => t11);

  u22 : g2 port map (
    a => l,
    b => q,
    y => t12);

  u23 : g2 port map (
    a => n,
    b => o,
    y => t13);

  u24 : g2 port map (
    a => p,
    b => m,
    y => t14);

  u25 : g2 port map (
    a => r,
    b => k,
    y => t15);

  u26 : g2 port map (
    a => t,
    b => i,
    y => t16);

  u27 : g6 port map (
    a => t11,
    b => t12,
    c => t13,
    d => t14,
    e => t15,
    f => t16,
    y => t17);

  u3 : g2 port map (
    a => d,
    b => p6,
    y => t2);

  u4 : inv port map (
    a => p6,
    y => c);

  u5 : g2 port map (
    a => t1,
    b => t2,
    y => g,
    y2 => t3);

  u6 : g2 port map (
    a => t3,
    b => e,
    y => t4);

  u7 : g2 port map (
    a => a,
    b => p25,
    y => t5);

  u8 : inv port map (
    a => p25,
    y => b);

  u9 : g2 port map (
    a => b,
    b => p28,
    y => t6);

p1 <= t10;
p23 <= t9;
p8 <= t17;
tp1 <= g;
tp2 <= t10;
tp3 <= t14;
tp5 <= t17;
tp6 <= t9;

end gates;


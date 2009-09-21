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
-- TQ module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tqslice is
    port (
      a : in std_logic;
      y : out std_logic;
      y1 : out std_logic;
      y2 : out std_logic);

end tqslice;
architecture gates of tqslice is
  component inv2
    port (
      a : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal t1 : std_logic;

begin -- gates
  u1 : inv2 port map (
    a => a,
    y => y,
    y2 => t1);

y1 <= t1;
y2 <= t1;

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tq is
    port (
      p12 : in std_logic;
      p13 : in std_logic;
      p18 : in std_logic;
      p2 : in std_logic;
      p21 : in std_logic;
      p24 : in std_logic;
      p27 : in std_logic;
      p5 : in std_logic;
      p7 : in std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p10 : out std_logic;
      p11 : out std_logic;
      p14 : out std_logic;
      p15 : out std_logic;
      p16 : out std_logic;
      p17 : out std_logic;
      p19 : out std_logic;
      p20 : out std_logic;
      p22 : out std_logic;
      p23 : out std_logic;
      p25 : out std_logic;
      p26 : out std_logic;
      p28 : out std_logic;
      p3 : out std_logic;
      p4 : out std_logic;
      p6 : out std_logic;
      p8 : out std_logic;
      p9 : out std_logic);

end tq;
architecture gates of tq is
  component inv2
    port (
      a : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component tqslice
    port (
      a : in std_logic;
      y : out std_logic;
      y1 : out std_logic;
      y2 : out std_logic);

  end component;

  signal t1 : std_logic;

begin -- gates
  u1 : tqslice port map (
    a => p12,
    y => tp1,
    y1 => p9,
    y2 => p11);

  u10 : inv2 port map (
    a => p18,
    y => tp5,
    y2 => t1);

  u2 : tqslice port map (
    a => p13,
    y => tp2,
    y1 => p14,
    y2 => p16);

  u3 : tqslice port map (
    a => p7,
    y => tp3,
    y1 => p8,
    y2 => p10);

  u4 : tqslice port map (
    a => p24,
    y => tp4,
    y1 => p25,
    y2 => p23);

  u5 : tqslice port map (
    a => p21,
    y => tp6,
    y1 => p22,
    y2 => p20);

  u6 : tqslice port map (
    a => p2,
    y1 => p1,
    y2 => p3);

  u7 : tqslice port map (
    a => p27,
    y1 => p28,
    y2 => p26);

  u8 : tqslice port map (
    a => p5,
    y1 => p4,
    y2 => p6);

p15 <= t1;
p17 <= t1;
p19 <= t1;

end gates;


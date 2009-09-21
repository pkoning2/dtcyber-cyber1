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
-- KE module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ke is
    port (
      p1 : in  std_logic;
      p10 : in  std_logic;
      p11 : in  std_logic;
      p13 : in  std_logic;
      p15 : in  std_logic;
      p17 : in  std_logic;
      p18 : in  std_logic;
      p19 : in  std_logic;
      p2 : in  std_logic;
      p20 : in  std_logic;
      p21 : in  std_logic;
      p22 : in  std_logic;
      p23 : in  std_logic;
      p25 : in  std_logic;
      p27 : in  std_logic;
      p3 : in  std_logic;
      p4 : in  std_logic;
      p5 : in  std_logic;
      p6 : in  std_logic;
      p8 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p12 : out std_logic;
      p14 : out std_logic;
      p16 : out std_logic;
      p24 : out std_logic;
      p28 : out std_logic;
      p7 : out std_logic;
      p9 : out std_logic);

end ke;
architecture gates of ke is
  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
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

  component g6
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      e : in  std_logic;
      f : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component inv
    port (
      a : in  std_logic;
      y : out std_logic);

  end component;

  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;
  signal t7 : std_logic;
  signal t8 : std_logic;

begin -- gates
  u1 : g2 port map (
    a => p3,
    b => p6,
    y => t1);

  u2 : inv port map (
    a => p2,
    y => t2);

  u3 : g2 port map (
    a => p1,
    b => t2,
    y => t3);

  u4 : g2 port map (
    a => p2,
    b => p4,
    y => t4);

  u5 : g6 port map (
    a => p8,
    b => p10,
    c => t1,
    d => p5,
    e => t3,
    f => t4,
    y => t5,
    y2 => p9);

  u6 : g4 port map (
    a => p27,
    b => p25,
    c => p23,
    d => p21,
    y => t6,
    y2 => p28);

  u7 : g4 port map (
    a => p22,
    b => p18,
    c => p19,
    d => p20,
    y => t7,
    y2 => p16);

  u8 : g4 port map (
    a => p17,
    b => p15,
    c => p11,
    d => p13,
    y => t8,
    y2 => p12);

p14 <= t8;
p24 <= t7;
p7 <= t5;
tp1 <= t5;
tp2 <= t6;
tp5 <= t7;
tp6 <= t8;

end gates;


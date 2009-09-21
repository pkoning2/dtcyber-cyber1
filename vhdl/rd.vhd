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
-- RD module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity rd is
    port (
      p1 : in  std_logic;
      p10 : in  std_logic;
      p12 : in  std_logic;
      p13 : in  std_logic;
      p14 : in  std_logic;
      p15 : in  std_logic;
      p17 : in  std_logic;
      p18 : in  std_logic;
      p19 : in  std_logic;
      p2 : in  std_logic;
      p20 : in  std_logic;
      p21 : in  std_logic;
      p22 : in  std_logic;
      p23 : in  std_logic;
      p24 : in  std_logic;
      p25 : in  std_logic;
      p28 : in  std_logic;
      p3 : in  std_logic;
      p4 : in  std_logic;
      p5 : in  std_logic;
      p8 : in  std_logic;
      p9 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p11 : out std_logic;
      p16 : out std_logic;
      p26 : out std_logic;
      p27 : out std_logic;
      p6 : out std_logic;
      p7 : out std_logic);

end rd;
architecture gates of rd is
  component inv
    port (
      a : in  std_logic;
      y : out std_logic);

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
  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;
  signal x : std_logic;

begin -- gates
  u1 : rsflop port map (
    r => x,
    s => p9,
    s2 => p3,
    s3 => p13,
    s4 => p1,
    q => t1);

  u2 : rsflop port map (
    r => x,
    s => p15,
    s2 => p17,
    s3 => p12,
    s4 => p14,
    q => t2);

  u3 : rsflop port map (
    r => a,
    s => p10,
    s2 => p8,
    q => t3);

  u4 : rsflop port map (
    r => x,
    s => p28,
    s2 => p24,
    s3 => p22,
    s4 => p20,
    q => t4);

  u5 : rsflop port map (
    r => a,
    s => p2,
    s2 => p4,
    q => t5);

  u6 : rsflop port map (
    r => x,
    s => p19,
    s2 => p21,
    s3 => p23,
    s4 => p25,
    q => t6);

  u7 : inv port map (
    a => p5,
    y => a);

  u8 : inv port map (
    a => p18,
    y => x);

p11 <= t1;
p16 <= t2;
p26 <= t4;
p27 <= t6;
p6 <= t5;
p7 <= t3;
tp1 <= t1;
tp2 <= t3;
tp3 <= t5;
tp4 <= t2;
tp5 <= t4;
tp6 <= t6;

end gates;


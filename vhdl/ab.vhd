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
-- AB module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity abslice is
    port (
      a : in std_logic;
      b : in std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic);

end abslice;
architecture gates of abslice is
  component g2
    port (
      a : in std_logic;
      b : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component inv
    port (
      a : in std_logic;
      y : out std_logic);

  end component;

  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);

  u2 : inv port map (
    a => a,
    y => t2);

  u3 : inv port map (
    a => b,
    y => t3);

  u4 : g2 port map (
    a => t2,
    b => t3,
    y => t4);

  u5 : g2 port map (
    a => t1,
    b => t4,
    y => t5);

q1 <= t5;
q2 <= t5;
q3 <= t5;

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ab is
    port (
      p1 : in std_logic;
      p11 : in std_logic;
      p13 : in std_logic;
      p18 : in std_logic;
      p20 : in std_logic;
      p23 : in std_logic;
      p24 : in std_logic;
      p26 : in std_logic;
      p28 : in std_logic;
      p3 : in std_logic;
      p5 : in std_logic;
      p8 : in std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p10 : out std_logic;
      p12 : out std_logic;
      p14 : out std_logic;
      p15 : out std_logic;
      p16 : out std_logic;
      p17 : out std_logic;
      p19 : out std_logic;
      p21 : out std_logic;
      p25 : out std_logic;
      p27 : out std_logic;
      p4 : out std_logic;
      p6 : out std_logic);

end ab;
architecture gates of ab is
  component abslice
    port (
      a : in std_logic;
      b : in std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic);

  end component;


begin -- gates
  u1 : abslice port map (
    a => p5,
    b => p8,
    q1 => p10,
    q2 => p12,
    q3 => tp1);

  u2 : abslice port map (
    a => p18,
    b => p20,
    q1 => p17,
    q2 => tp4,
    q3 => p15);

  u3 : abslice port map (
    a => p1,
    b => p3,
    q1 => p4,
    q2 => tp2,
    q3 => p6);

  u4 : abslice port map (
    a => p28,
    b => p26,
    q1 => p27,
    q2 => tp5,
    q3 => p25);

  u5 : abslice port map (
    a => p13,
    b => p11,
    q1 => p14,
    q2 => tp3,
    q3 => p16);

  u6 : abslice port map (
    a => p24,
    b => p23,
    q1 => tp6,
    q2 => p21,
    q3 => p19);


end gates;


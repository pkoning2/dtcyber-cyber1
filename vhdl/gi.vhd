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
-- GI module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity gi is
    port (
      p2 : in  std_logic;
      p3 : in  std_logic;
      p5 : in  std_logic;
      p7 : in  std_logic;
      p9 : in  std_logic;
      p11 : in  std_logic;
      p13 : in  std_logic;
      p16 : in  std_logic;
      p18 : in  std_logic;
      p20 : in  std_logic;
      p22 : in  std_logic;
      p24 : in  std_logic;
      p26 : in  std_logic;
      p27 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p4 : out std_logic;
      p6 : out std_logic;
      p8 : out std_logic;
      p10 : out std_logic;
      p12 : out std_logic;
      p14 : out std_logic;
      p15 : out std_logic;
      p17 : out std_logic;
      p19 : out std_logic;
      p21 : out std_logic;
      p23 : out std_logic;
      p25 : out std_logic;
      p28 : out std_logic);

end gi;
architecture gates of gi is
  component inv2
    port (
      a : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;

begin -- gates
  u1 : inv2 port map (
    a => p2,
    y2 => p1);


  u2 : inv2 port map (
    a => p3,
    y2 => p4);


  u3 : inv2 port map (
    a => p5,
    y2 => p6);


  u4 : inv2 port map (
    a => p7,
    y2 => p8);


  u5 : inv2 port map (
    a => p9,
    y2 => t1);

  p10 <= t1;
  tp3 <= t1;

  u6 : inv2 port map (
    a => p11,
    y2 => t2);

  p12 <= t2;
  tp2 <= t2;

  u7 : inv2 port map (
    a => p13,
    y2 => t3);

  p14 <= t3;
  tp1 <= t3;

  u8 : inv2 port map (
    a => p16,
    y2 => t4);

  p15 <= t4;
  tp6 <= t4;

  u9 : inv2 port map (
    a => p18,
    y2 => t5);

  p17 <= t5;
  tp5 <= t5;

  u10 : inv2 port map (
    a => p20,
    y2 => t6);

  p19 <= t6;
  tp4 <= t6;

  u11 : inv2 port map (
    a => p22,
    y2 => p21);


  u12 : inv2 port map (
    a => p24,
    y2 => p23);


  u13 : inv2 port map (
    a => p26,
    y2 => p25);


  u14 : inv2 port map (
    a => p27,
    y2 => p28);



end gates;


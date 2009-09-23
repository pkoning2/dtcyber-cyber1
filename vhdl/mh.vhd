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
-- MH module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity mh is
    port (
      p1 : in  std_logic;
      p2 : in  std_logic;
      p3 : in  std_logic;
      p4 : in  std_logic;
      p8 : in  std_logic;
      p10 : in  std_logic;
      p11 : in  std_logic;
      p17 : in  std_logic;
      p21 : in  std_logic;
      p23 : in  std_logic;
      p25 : in  std_logic;
      p26 : in  std_logic;
      p27 : in  std_logic;
      p28 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p5 : out std_logic;
      p6 : out std_logic;
      p7 : out std_logic;
      p9 : out std_logic;
      p12 : out std_logic;
      p13 : out std_logic;
      p14 : out std_logic;
      p15 : out std_logic;
      p16 : out std_logic;
      p18 : out std_logic;
      p19 : out std_logic;
      p20 : out std_logic;
      p22 : out std_logic;
      p24 : out std_logic);

end mh;
architecture gates of mh is
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
  signal f : std_logic;
  signal g : std_logic;
  signal h : std_logic;
  signal i : std_logic;
  signal j : std_logic;
  signal k : std_logic;
  signal l : std_logic;
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
    a => b,
    b => p10,
    y => t1);

  p5 <= t1;
  p6 <= t1;
  tp1 <= t1;

  u2 : g3 port map (
    a => d,
    b => e,
    c => f,
    y => t2);

  p7 <= t2;
  p9 <= t2;
  p14 <= t2;
  tp2 <= t2;

  u3 : inv port map (
    a => p8,
    y => d);


  u4 : inv port map (
    a => p1,
    y => a);


  u5 : g2 port map (
    a => p1,
    b => p3,
    y => e);


  u6 : g3 port map (
    a => p3,
    b => p2,
    c => p4,
    y => f);


  u7 : inv2 port map (
    a => p2,
    y => b,
    y2 => t3);


  u8 : g2 port map (
    a => t3,
    b => p4,
    y => c);


  u9 : g3 port map (
    a => a,
    b => c,
    c => p11,
    y => t4);

  p12 <= t4;
  p13 <= t4;
  tp3 <= t4;

  u10 : g2 port map (
    a => h,
    b => p21,
    y => t5);

  p22 <= t5;
  p24 <= t5;
  tp6 <= t5;

  u11 : g3 port map (
    a => j,
    b => k,
    c => l,
    y => t6);

  p15 <= t6;
  p19 <= t6;
  p20 <= t6;
  tp5 <= t6;

  u12 : inv port map (
    a => p23,
    y => j);


  u13 : inv port map (
    a => p28,
    y => g);


  u14 : g2 port map (
    a => p28,
    b => p26,
    y => k);


  u15 : g3 port map (
    a => p26,
    b => p27,
    c => p25,
    y => l);


  u16 : inv2 port map (
    a => p27,
    y => h,
    y2 => t7);


  u17 : g2 port map (
    a => t7,
    b => p25,
    y => i);


  u18 : g3 port map (
    a => g,
    b => i,
    c => p17,
    y => t8);

  p16 <= t8;
  p18 <= t8;
  tp4 <= t8;


end gates;


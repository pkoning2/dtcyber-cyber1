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
-- MI module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity mi is
    port (
      p2 : in  std_logic;
      p4 : in  std_logic;
      p5 : in  std_logic;
      p6 : in  std_logic;
      p7 : in  std_logic;
      p8 : in  std_logic;
      p17 : in  std_logic;
      p19 : in  std_logic;
      p21 : in  std_logic;
      p23 : in  std_logic;
      p25 : in  std_logic;
      p27 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p3 : out std_logic;
      p9 : out std_logic;
      p10 : out std_logic;
      p11 : out std_logic;
      p13 : out std_logic;
      p14 : out std_logic;
      p16 : out std_logic;
      p20 : out std_logic;
      p22 : out std_logic;
      p24 : out std_logic;
      p26 : out std_logic;
      p28 : out std_logic);

end mi;
architecture gates of mi is
  component g3
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
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

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal d : std_logic;
  signal e : std_logic;
  signal f : std_logic;
  signal g : std_logic;
  signal h : std_logic;
  signal i : std_logic;
  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;

begin -- gates
  u1 : g6 port map (
    a => g,
    b => h,
    c => i,
    d => p6,
    e => p4,
    f => p2,
    y => t1,
    y2 => p9);

  p3 <= t1;
  p10 <= h;
  tp1 <= t1;

  u2 : g3 port map (
    a => g,
    b => h,
    c => i,
    y => t2);

  p1 <= t2;
  tp2 <= t2;

  u3 : g6 port map (
    a => d,
    b => e,
    c => f,
    d => g,
    e => h,
    f => i,
    y => t3,
    y2 => p11);

  p16 <= e;
  tp3 <= t3;

  u4 : g3 port map (
    a => a,
    b => b,
    c => c,
    y => t4);

  p14 <= t4;
  p20 <= c;
  p24 <= b;
  p26 <= b;
  tp4 <= t4;

  u5 : g3 port map (
    a => d,
    b => e,
    c => f,
    y => t5);

  p13 <= t5;
  tp5 <= t5;

  u6 : g6 port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    f => f,
    y => t6,
    y2 => p28);

  p22 <= t6;
  tp6 <= t6;

  u7 : inv port map (
    a => p7,
    y => g);


  u8 : inv port map (
    a => p8,
    y => h);


  u9 : inv port map (
    a => p5,
    y => i);


  u10 : inv port map (
    a => p23,
    y => d);


  u11 : inv port map (
    a => p17,
    y => e);


  u12 : inv port map (
    a => p19,
    y => f);


  u13 : inv port map (
    a => p21,
    y => a);


  u14 : inv port map (
    a => p27,
    y => b);


  u15 : inv port map (
    a => p25,
    y => c);



end gates;


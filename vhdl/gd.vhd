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
-- GD module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity gd is
    port (
      p8 : in  std_logic;
      p9 : in  std_logic;
      p10 : in  std_logic;
      p13 : in  std_logic;
      p16 : in  std_logic;
      p17 : in  std_logic;
      p18 : in  std_logic;
      p20 : in  std_logic;
      p22 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p4 : out std_logic;
      p5 : out std_logic;
      p12 : out std_logic;
      p15 : out std_logic;
      p19 : out std_logic;
      p21 : out std_logic;
      p24 : out std_logic;
      p26 : out std_logic;
      p27 : out std_logic;
      p28 : out std_logic);

end gd;
architecture gates of gd is
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

  component g4
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
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
  signal m : std_logic;
  signal n : std_logic;
  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;
  signal t7 : std_logic;
  signal t8 : std_logic;
  signal t9 : std_logic;
  signal t10 : std_logic;
  signal t11 : std_logic;

begin -- gates
  u1 : g2 port map (
    a => p18,
    b => p10,
    y => j);


  u2 : g3 port map (
    a => p10,
    b => p18,
    c => i,
    y => m);

  tp1 <= i;

  u3 : inv port map (
    a => p20,
    y => n);


  u4 : inv2 port map (
    a => p9,
    y => t1,
    y2 => k);


  u5 : inv port map (
    a => p9,
    y => i);


  u6 : g2 port map (
    a => t1,
    b => i,
    y => l);


  u7 : g3 port map (
    a => l,
    b => m,
    c => n,
    y => t2);


  u8 : g2 port map (
    a => j,
    b => k,
    y => t3);


  u9 : g2 port map (
    a => p8,
    b => t3,
    y => e);


  u10 : g3 port map (
    a => e,
    b => f,
    c => a,
    y => t4);

  p5 <= t4;
  tp2 <= t4;

  u11 : g3 port map (
    a => i,
    b => k,
    c => j,
    y => f);


  u12 : g2 port map (
    a => t2,
    b => p22,
    y => g);


  u13 : inv port map (
    a => p22,
    y => t5);

  p24 <= t5;
  tp5 <= t5;

  u14 : g4 port map (
    a => t5,
    b => n,
    c => m,
    d => l,
    y => h);


  u15 : g3 port map (
    a => g,
    b => h,
    c => a,
    y => t6);

  p26 <= t6;
  tp6 <= t6;

  u16 : inv port map (
    a => p16,
    y => a);


  u17 : g2 port map (
    a => p13,
    b => p17,
    y => c);


  u18 : inv port map (
    a => p17,
    y => t7);

  p15 <= t7;
  tp4 <= t7;

  u19 : inv port map (
    a => p13,
    y => t8);


  u20 : g2 port map (
    a => t7,
    b => t8,
    y => d);


  u21 : g3 port map (
    a => a,
    b => c,
    c => d,
    y => p19);


  u22 : g3 port map (
    a => a,
    b => e,
    c => f,
    y => t9);

  p1 <= t9;
  p4 <= t9;

  u23 : g3 port map (
    a => a,
    b => g,
    c => h,
    y => t10);

  p27 <= t10;
  p28 <= t10;

  u24 : g3 port map (
    a => a,
    b => c,
    c => d,
    y => t11);

  p12 <= t11;
  p21 <= t11;
  tp3 <= t11;


end gates;


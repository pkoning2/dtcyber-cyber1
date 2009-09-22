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
-- FD module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity fd is
    port (
      p1 : in  std_logic;
      p5 : in  std_logic;
      p7 : in  std_logic;
      p9 : in  std_logic;
      p11 : in  std_logic;
      p13 : in  std_logic;
      p17 : in  std_logic;
      p18 : in  std_logic;
      p19 : in  std_logic;
      p20 : in  std_logic;
      p21 : in  std_logic;
      p22 : in  std_logic;
      p23 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p6 : out std_logic;
      p10 : out std_logic;
      p12 : out std_logic;
      p15 : out std_logic;
      p24 : out std_logic;
      p26 : out std_logic;
      p27 : out std_logic);

end fd;
architecture gates of fd is
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

  component g5
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      e : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component inv
    port (
      a : in  std_logic;
      y : out std_logic);

  end component;

  signal a : std_logic;
  signal c : std_logic;
  signal d : std_logic;
  signal e : std_logic;
  signal f : std_logic;
  signal g : std_logic;
  signal h : std_logic;
  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;

begin -- gates
  u1 : inv port map (
    a => p9,
    y => p15);


  u2 : g2 port map (
    a => p9,
    b => p13,
    y => c);

  tp2 <= c;

  u3 : inv port map (
    a => p11,
    y => d);

  tp1 <= d;

  u4 : inv port map (
    a => p18,
    y => a);

  p27 <= a;

  u5 : g2 port map (
    a => p17,
    b => a,
    y => e);

  tp5 <= e;

  u6 : inv port map (
    a => p20,
    y => t1);


  u7 : g3 port map (
    a => a,
    b => p19,
    c => t1,
    y => f);

  tp4 <= f;

  u8 : g3 port map (
    a => p21,
    b => a,
    c => p23,
    y => g);

  tp6 <= g;

  u9 : g5 port map (
    a => c,
    b => d,
    c => e,
    d => f,
    e => g,
    y => t2,
    y2 => t4);

  p10 <= t4;
  p12 <= t4;
  tp3 <= t2;

  u10 : g2 port map (
    a => t2,
    b => p22,
    y => p26);


  u11 : g2 port map (
    a => t2,
    b => h,
    y => p6);


  u12 : g3 port map (
    a => p5,
    b => p1,
    c => p7,
    y2 => t3);


  u13 : g2 port map (
    a => t2,
    b => t3,
    y => p24);


  u14 : g2 port map (
    a => p1,
    b => p7,
    y2 => h);



end gates;


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
-- KS module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ksslice is
    port (
      a : in  std_logic;
      e : in  std_logic;
      f : in  std_logic;
      q : out std_logic);

end ksslice;
architecture gates of ksslice is
  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
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

begin -- gates
  u1 : g2 port map (
    a => f,
    b => a,
    y => t1);


  u2 : inv port map (
    a => a,
    y => t2);


  u3 : g2 port map (
    a => t2,
    b => e,
    y => t3);


  u4 : g2 port map (
    a => t1,
    b => t3,
    y => q);



end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ks is
    port (
      p1 : in  std_logic;
      p4 : in  std_logic;
      p7 : in  std_logic;
      p10 : in  std_logic;
      p12 : in  std_logic;
      p14 : in  std_logic;
      p18 : in  std_logic;
      p19 : in  std_logic;
      p21 : in  std_logic;
      p22 : in  std_logic;
      p25 : in  std_logic;
      p28 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p2 : out std_logic;
      p3 : out std_logic;
      p5 : out std_logic;
      p6 : out std_logic;
      p8 : out std_logic;
      p9 : out std_logic;
      p11 : out std_logic;
      p13 : out std_logic;
      p15 : out std_logic;
      p16 : out std_logic;
      p20 : out std_logic;
      p23 : out std_logic;
      p24 : out std_logic;
      p26 : out std_logic;
      p27 : out std_logic);

end ks;
architecture gates of ks is
  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
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

  component inv2
    port (
      a : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component ksslice
    port (
      a : in  std_logic;
      e : in  std_logic;
      f : in  std_logic;
      q : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal d : std_logic;
  signal e : std_logic;
  signal f : std_logic;
  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;

begin -- gates
  u1 : ksslice port map (
    a => p28,
    e => e,
    f => f,
    q => a);

  p8 <= f;
  p27 <= a;

  u2 : ksslice port map (
    a => p25,
    e => e,
    f => f,
    q => b);

  p26 <= b;
  tp6 <= b;

  u3 : ksslice port map (
    a => p1,
    e => e,
    f => f,
    q => c);

  p2 <= c;
  tp3 <= c;

  u4 : ksslice port map (
    a => p4,
    e => e,
    f => f,
    q => d);

  p3 <= d;
  tp1 <= d;

  u5 : ksslice port map (
    a => p10,
    e => e,
    f => f,
    q => t1);

  p5 <= t1;
  p9 <= t1;
  tp2 <= t1;

  u6 : inv port map (
    a => t1,
    y => p6);


  u7 : ksslice port map (
    a => p22,
    e => e,
    f => f,
    q => t2);

  p23 <= t2;
  p24 <= t2;
  tp5 <= t2;

  u8 : g6 port map (
    a => p18,
    b => a,
    c => b,
    d => c,
    e => d,
    f => p12,
    y => tp4,
    y2 => t3);

  p11 <= t3;
  p20 <= t3;

  u9 : g2 port map (
    a => p21,
    b => p19,
    y => p15);


  u10 : g2 port map (
    a => p14,
    b => p19,
    y => p16,
    y2 => p13);


  u11 : inv2 port map (
    a => p7,
    y => e,
    y2 => f);



end gates;


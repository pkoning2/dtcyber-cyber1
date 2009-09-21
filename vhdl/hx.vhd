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
-- HX module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity hx is
    port (
      p1 : in std_logic;
      p10 : in std_logic;
      p13 : in std_logic;
      p15 : in std_logic;
      p16 : in std_logic;
      p18 : in std_logic;
      p19 : in coaxsig;
      p2 : in std_logic;
      p20 : in std_logic;
      p21 : in std_logic;
      p23 : in std_logic;
      p25 : in std_logic;
      p26 : in std_logic;
      p27 : in std_logic;
      p28 : in std_logic;
      p3 : in std_logic;
      p4 : in std_logic;
      p5 : in std_logic;
      p7 : in std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p11 : out std_logic;
      p12 : out std_logic;
      p14 : out std_logic;
      p17 : out std_logic;
      p22 : out std_logic;
      p24 : out std_logic;
      p6 : out std_logic;
      p8 : out std_logic;
      p9 : out std_logic);

end hx;
architecture gates of hx is
  component cxreceiver
    port (
      a : in coaxsig;
      y : out std_logic);

  end component;

  component g2
    port (
      a : in std_logic;
      b : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component g3
    port (
      a : in std_logic;
      b : in std_logic;
      c : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component g6
    port (
      a : in std_logic;
      b : in std_logic;
      c : in std_logic;
      d : in std_logic;
      e : in std_logic;
      f : in std_logic;
      y : out std_logic;
      y2 : out std_logic);

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
  signal b : std_logic;
  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;

begin -- gates
  u1 : cxreceiver port map (
    a => p19,
    y => t1);

  u10 : g6 port map (
    a => p21,
    b => p23,
    c => p26,
    d => p25,
    e => p28,
    f => p27,
    y => tp5,
    y2 => t6);

  u2 : rsflop port map (
    r => p20,
    s => t1,
    q => t2);

  u3 : g6 port map (
    a => p5,
    b => p7,
    c => t2,
    d => p3,
    e => p1,
    f => p2,
    y => a);

  u4 : rsflop port map (
    r => p10,
    s => a,
    q => t3,
    qb => b);

  u5 : g2 port map (
    a => p4,
    b => t3,
    y => p6);

  u6 : g3 port map (
    a => p15,
    b => p16,
    c => p18,
    y => t4);

  u7 : g2 port map (
    a => p16,
    b => p18,
    y => p17);

  u8 : g2 port map (
    a => a,
    b => b,
    y => t5);

  u9 : g2 port map (
    a => p13,
    b => t5,
    y => p11);

p12 <= t4;
p14 <= t5;
p22 <= t6;
p24 <= t6;
p8 <= t3;
p9 <= a;
tp1 <= t3;
tp2 <= a;
tp3 <= t5;
tp4 <= t4;
tp6 <= t2;

end gates;


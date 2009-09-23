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
-- MJ module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity mj is
    port (
      p3 : in  std_logic;
      p5 : in  std_logic;
      p6 : in  std_logic;
      p7 : in  std_logic;
      p8 : in  std_logic;
      p9 : in  std_logic;
      p10 : in  std_logic;
      p11 : in  std_logic;
      p12 : in  std_logic;
      p16 : in  std_logic;
      p17 : in  std_logic;
      p18 : in  std_logic;
      p19 : in  std_logic;
      p20 : in  std_logic;
      p21 : in  std_logic;
      p27 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p2 : out std_logic;
      p14 : out std_logic;
      p15 : out std_logic;
      p22 : out std_logic;
      p24 : out std_logic;
      p26 : out std_logic;
      p28 : out std_logic);

end mj;
architecture gates of mj is
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
  signal t12 : std_logic;
  signal t13 : std_logic;
  signal t14 : std_logic;
  signal t15 : std_logic;
  signal t16 : std_logic;
  signal t17 : std_logic;
  signal t18 : std_logic;

begin -- gates
  u1 : inv port map (
    a => p11,
    y => t1);


  u2 : g2 port map (
    a => a,
    b => p17,
    y => t2);


  u3 : g3 port map (
    a => a,
    b => b,
    c => p8,
    y => t3);


  u4 : g3 port map (
    a => a,
    b => c,
    c => p9,
    y => t4);


  u5 : g4 port map (
    a => a,
    b => b,
    c => p10,
    d => d,
    y => t5);


  u6 : g4 port map (
    a => c,
    b => p12,
    c => p6,
    d => a,
    y => t6);


  u7 : g6 port map (
    a => t1,
    b => t2,
    c => t3,
    d => t4,
    e => t5,
    f => t6,
    y => t7,
    y2 => t8);

  tp2 <= t7;

  u8 : inv port map (
    a => t8,
    y => t9);

  p2 <= t9;
  p15 <= t9;
  tp1 <= t9;

  u9 : g2 port map (
    a => t7,
    b => p7,
    y => p14);


  u10 : g3 port map (
    a => p7,
    b => t7,
    c => p3,
    y => p1);


  u11 : g4 port map (
    a => b,
    b => p11,
    c => d,
    d => p19,
    y => t10);


  u12 : inv port map (
    a => p17,
    y => t11);


  u13 : g2 port map (
    a => p8,
    b => b,
    y => t12);


  u14 : g2 port map (
    a => p9,
    b => c,
    y => t13);


  u15 : g3 port map (
    a => p10,
    b => b,
    c => d,
    y => t14);


  u16 : g3 port map (
    a => p12,
    b => p6,
    c => c,
    y => t15);


  u17 : g6 port map (
    a => t10,
    b => t11,
    c => t12,
    d => t13,
    e => t14,
    f => t15,
    y => t16,
    y2 => t17);

  tp5 <= t16;

  u18 : g3 port map (
    a => p18,
    b => t16,
    c => p16,
    y => p28);


  u19 : g2 port map (
    a => p16,
    b => t16,
    y => p26);


  u20 : inv port map (
    a => t17,
    y => t18);

  p22 <= t18;
  p24 <= t18;
  tp6 <= t18;

  u21 : inv port map (
    a => p5,
    y => a);


  u22 : inv port map (
    a => p21,
    y => b);


  u23 : inv port map (
    a => p20,
    y => c);


  u24 : inv port map (
    a => p27,
    y => d);



end gates;


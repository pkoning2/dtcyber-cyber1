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
-- ME module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity me is
    port (
      p1 : in  std_logic;
      p2 : in  std_logic;
      p3 : in  std_logic;
      p5 : in  std_logic;
      p10 : in  std_logic;
      p11 : in  std_logic;
      p15 : in  std_logic;
      p21 : in  std_logic;
      p24 : in  std_logic;
      p25 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p6 : out std_logic;
      p7 : out std_logic;
      p8 : out std_logic;
      p9 : out std_logic;
      p12 : out std_logic;
      p13 : out std_logic;
      p19 : out std_logic;
      p20 : out std_logic;
      p23 : out std_logic;
      p27 : out std_logic);

end me;
architecture gates of me is
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
  signal t19 : std_logic;
  signal t20 : std_logic;
  signal t21 : std_logic;

begin -- gates
  u1 : g3 port map (
    a => c,
    b => p2,
    c => a,
    y => t1);


  u2 : g2 port map (
    a => a,
    b => b,
    y => t2);


  u3 : g2 port map (
    a => c,
    b => b,
    y => t3);


  u4 : g3 port map (
    a => t1,
    b => t2,
    c => t3,
    y => t4);

  p6 <= t4;
  tp1 <= t4;

  u5 : g3 port map (
    a => c,
    b => d,
    c => f,
    y => t5);


  u6 : g2 port map (
    a => d,
    b => e,
    y => t6);


  u7 : g2 port map (
    a => e,
    b => f,
    y => t7);


  u8 : g3 port map (
    a => t5,
    b => t6,
    c => t7,
    y => t8);

  p9 <= t8;
  tp3 <= t8;

  u9 : g2 port map (
    a => p2,
    b => b,
    y => t9);


  u10 : g2 port map (
    a => a,
    b => c,
    y => t10);


  u11 : g2 port map (
    a => t9,
    b => t10,
    y => t11,
    y2 => p8);

  p7 <= t11;
  tp2 <= t11;

  u12 : g2 port map (
    a => c,
    b => e,
    y => t12);


  u13 : g2 port map (
    a => d,
    b => f,
    y => t13);


  u14 : g2 port map (
    a => t12,
    b => t13,
    y => t14,
    y2 => p13);

  p12 <= t14;
  tp4 <= t14;

  u15 : g2 port map (
    a => f,
    b => h,
    y => t15);


  u16 : g2 port map (
    a => g,
    b => i,
    y => t16);

  p27 <= i;

  u17 : g2 port map (
    a => t15,
    b => t16,
    y => t17,
    y2 => p20);

  p23 <= t17;
  tp5 <= t17;

  u18 : g3 port map (
    a => f,
    b => i,
    c => g,
    y => t18);


  u19 : g2 port map (
    a => g,
    b => h,
    y => t19);


  u20 : g2 port map (
    a => h,
    b => i,
    y => t20);


  u21 : g3 port map (
    a => t18,
    b => t19,
    c => t20,
    y => t21);

  p19 <= t21;
  tp6 <= t21;

  u22 : inv port map (
    a => p3,
    y => a);


  u23 : inv port map (
    a => p5,
    y => b);


  u24 : inv port map (
    a => p1,
    y => c);


  u25 : inv port map (
    a => p11,
    y => d);


  u26 : inv port map (
    a => p10,
    y => e);


  u27 : inv port map (
    a => p15,
    y => f);


  u28 : inv port map (
    a => p24,
    y => g);


  u29 : inv port map (
    a => p21,
    y => h);


  u30 : inv port map (
    a => p25,
    y => i);



end gates;


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
-- MA module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ma is
    port (
      p2 : in  std_logic;
      p10 : in  std_logic;
      p11 : in  std_logic;
      p13 : in  std_logic;
      p15 : in  std_logic;
      p27 : in  std_logic;
      tp1 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p3 : out std_logic;
      p5 : out std_logic;
      p6 : out std_logic;
      p7 : out std_logic;
      p8 : out std_logic;
      p9 : out std_logic;
      p12 : out std_logic;
      p14 : out std_logic;
      p16 : out std_logic;
      p17 : out std_logic;
      p18 : out std_logic;
      p19 : out std_logic;
      p21 : out std_logic;
      p23 : out std_logic;
      p25 : out std_logic;
      p28 : out std_logic);

end ma;
architecture gates of ma is
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
  signal n : std_logic;
  signal o : std_logic;
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

begin -- gates
  u1 : rsflop port map (
    r => n,
    s => p11,
    q => a,
    qb => b);

  p6 <= n;
  p14 <= n;
  tp1 <= a;

  u2 : inv port map (
    a => a,
    y => c);

  p12 <= c;
  p28 <= c;

  u3 : inv port map (
    a => b,
    y => d);

  p3 <= d;

  u4 : rsflop port map (
    r => n,
    s => p10,
    q => e,
    qb => f);

  tp5 <= e;

  u5 : inv port map (
    a => e,
    y => g);

  p9 <= g;
  p23 <= g;

  u6 : inv port map (
    a => f,
    y => h);

  p7 <= h;

  u7 : rsflop port map (
    r => n,
    s => p15,
    q => i,
    qb => j);

  p16 <= j;
  tp6 <= i;

  u8 : inv port map (
    a => i,
    y => k);

  p18 <= k;

  u9 : inv port map (
    a => j,
    y => l);

  p17 <= l;

  u10 : inv port map (
    a => p2,
    y => o);


  u11 : g2 port map (
    a => p2,
    b => d,
    y => t1);


  u12 : g2 port map (
    a => c,
    b => o,
    y => t2);


  u13 : g2 port map (
    a => t1,
    b => t2,
    y => p1);


  u14 : g2 port map (
    a => d,
    b => h,
    y => t3);


  u15 : g2 port map (
    a => g,
    b => b,
    y => t4);


  u16 : g2 port map (
    a => t3,
    b => t4,
    y => p5);


  u17 : g2 port map (
    a => h,
    b => i,
    y => t5);


  u18 : g2 port map (
    a => k,
    b => f,
    y => t6);


  u19 : g2 port map (
    a => t5,
    b => t6,
    y => p8);


  u20 : inv port map (
    a => k,
    y => p19);


  u21 : inv port map (
    a => p13,
    y => n);


  u22 : g2 port map (
    a => l,
    b => h,
    y => t7);


  u23 : g2 port map (
    a => d,
    b => p27,
    y => t8,
    y2 => t9);


  u24 : g2 port map (
    a => l,
    b => t9,
    y => t10);


  u25 : inv port map (
    a => p27,
    y => t11);


  u26 : g2 port map (
    a => b,
    b => t11,
    y => t12);


  u27 : g2 port map (
    a => t12,
    b => h,
    y => t13);


  u28 : g3 port map (
    a => t7,
    b => t10,
    c => t13,
    y => p21);


  u29 : g2 port map (
    a => t13,
    b => t8,
    y => p25);



end gates;


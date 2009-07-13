-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- PM module, rev C
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pm is
  
  port (
    p8, p6, p10, p4                 : in std_logic;
    p17, p13, p23, p21, p3, p5, p25 : in std_logic;
    p26, p24, p22                   : in std_logic;
    tp1, tp2, tp5, tp6              : out std_logic;  -- test points
    p1, p2, p7, p11, p16, p9        : out std_logic;
    p20, p19, p12, p14              : out std_logic;
    p18, p15, p28, p27              : out std_logic);
  
end pm;

architecture gates of pm is
  component inv
    port (
      a  : in  std_logic;                     -- input
      y  : out std_logic);                    -- output
  end component;
  component inv2
    port (
      a  : in  std_logic;                     -- input
      y, y2 : out std_logic);                    -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component g3
    port (
      a, b, c : in  std_logic;                -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component g4
    port (
      a, b, c, d : in  std_logic;             -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal a, b, c, d, e, f, g, h : std_logic;
  signal j, k, l, m, n, r, t : std_logic;
  signal t1, t2, t3, t4, t5, t6 : std_logic;
begin  -- gates

  u1 : inv port map (
    a => p17,
    y => j);
  u2 : inv port map (
    a => p13,
    y => k);
  u3 : inv port map (
    a => p23,
    y => l);
  u4 : inv port map (
    a => p21,
    y => m);
  u5 : inv port map (
    a => p3,
    y => n);
  u6 : inv port map (
    a => p5,
    y => r);
  u7 : inv port map (
    a => p25,
    y => t);
  u8 : g3 port map (
    a => j,
    b => p8,
    c => r,
    y => e);
  u9 : g3 port map (
    a => k,
    b => p6,
    c => r,
    y => f);
  u10 : g2 port map (
    a  => e,
    b  => f,
    y  => t1,
    y2 => p2);
  tp1 <= t1;
  p1 <= t1;
  u11 : inv2 port map (
    a  => f,
    y2 => p7);
  u12 : g4 port map (
    a  => a,
    b  => c,
    c  => e,
    d  => g,
    y2 => t2);
  p20 <= t2;
  p19 <= t2;
  u13 : g3 port map (
    a => j,
    b => p10,
    c => n,
    y => c);
  u14 : g3 port map (
    a => k,
    b => p4,
    c => n,
    y => d);
  u15 : g2 port map (
    a  => c,
    b  => d,
    y  => t3,
    y2 => p16);
  tp2 <= t3;
  p11 <= t3;
  u16 : inv2 port map (
    a  => d,
    y2 => p9);
  u17 : g4 port map (
    a  => b,
    b  => d,
    c  => f,
    d  => h,
    y2 => t4);
  p12 <= t4;
  p14 <= t4;
  u18 : g3 port map (
    a => k,
    b => p26,
    c => t,
    y => h);
  u19 : g3 port map (
    a => t,
    b => j,
    c => p24,
    y => g);
  u20 : g2 port map (
    a  => h,
    b  => g,
    y  => t5,
    y2 => p18);
  tp5 <= t5;
  p15 <= t5;
  u21 : g4 port map (
    a => p24,
    b => k,
    c => l,
    d => m,
    y => b);
  u22 : g4 port map (
    a => j,
    b => l,
    c => m,
    d => p22,
    y => a);
  u23 : g2 port map (
    a  => b,
    b  => a,
    y  => t6,
    y2 => p28);
  tp6 <= t6;
  p27 <= t6;
  
end gates;

-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell, Al Kossow
--
-- Based on the original design by Seymour Cray and his team
--
-- PF module rev C
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pf is
  
  port (
    p2, p15, p16, p3, p1                   : in  std_logic;
    p5, p26, p4, p27, p28                  : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p17, p11, p20, p10, p19, p13, p24, p12 : out std_logic;
    p21, p9, p22, p14, p18                 : out std_logic);

end pf;

architecture gates of pf is
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
  component rsflop
    port (
      s, r  : in  std_logic;                  -- set, reset
      s2, s3, s4, r2, r3, r4  : in  std_logic := '1';-- extra set, reset if needed
      q, qb : out std_logic);                 -- q and q.bar
  end component;
  signal a, i, e, d, g, k, c, j, w, x : std_logic;
  signal t1, t2, t3, t4, t5, t6, t7 : std_logic;
begin  -- gates

  u1 : inv port map (
    a => p2,
    y => j);
  u2 : g2 port map (
    a  => j,
    b  => p5,
    y2 => d);
  u3 : g2 port map (
    a  => j,
    b  => p26,
    y2 => g);
  u4 : g2 port map (
    a  => j,
    b  => p4,
    y2 => k);
  u5 : g3 port map (
    a  => j,
    b  => p28,
    c  => p27,
    y2 => c);
  u6 : g3 port map (
    a => a,
    b => e,
    c => c,
    y => w);
  p17 <= w;
  p19 <= w;
  tp5 <= w;
  p21 <= w;
  u7 : g3 port map (
    a => a,
    b => e,
    c => d,
    y => x);
  p11 <= x;
  p13 <= x;
  tp2 <= x;
  p9 <= x;
  u8 : g2 port map (
    a => i,
    b => g,
    y => t1);
  p20 <= t1;
  p24 <= t1;
  tp4 <= t1;
  p22 <= t1;
  u9 : g2 port map (
    a => i,
    b => k,
    y => t2);
  p10 <= t2;
  p12 <= t2;
  tp3 <= t2;
  p14 <= t2;
  u10 : inv2 port map (
    a  => p3,
    y  => i,
    y2 => a);
  tp1 <= a;
  u11 : inv port map (
    a => p1,
    y => e);
  u12 : g2 port map (
    a => x,
    b => w,
    y => t3);
  u13 : inv2 port map (
    a  => p15,
    y2 => t4);
  u14 : g2 port map (
    a => t3,
    b => t4,
    y => t5);
  u15 : inv port map (
    a => p16,
    y => t6);
  u16 : rsflop port map (
    s => t5,
    r => t6,
    q => t7);
  tp6 <= t7;
  u17 : inv port map (
    a => t7,
    y => p18);
end gates;

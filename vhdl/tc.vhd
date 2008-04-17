-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity tc is
  
  port (
    p3, p5, p14, p16            : in  std_logic := '1';  -- inputs
    tp1, tp5                    : out std_logic;  -- test point 1 (stage 1)
    tp2, tp6                    : out std_logic;  -- test point 2 (stage 2)
    p1, p2, p4, p6, p7, p8      : out std_logic;  -- outputs 1
    p9, p10, p11, p12, p13, p15 : out std_logic;  -- outputs 1
    p17, p18, p19, p20, p21, p22 : out std_logic;  -- outputs 2
    p23, p24, p25, p26, p27, p28 : out std_logic);  -- outputs 2

end tc;

architecture gates of tc is
  component inv
    port (
      i : in  std_logic;                      -- input
      o : out std_logic);                     -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      x    : out std_logic);                  -- output
  end component;
  signal a, b, c, d, e, f : std_logic;
begin  -- gates
  u1 : g2 port map (
    a => p3,
    b => p5,
    x => a);
  tp1 <= a;
  u2 : inv port map (
    i => a,
    o => b);
  tp2 <= b;
  u3 : inv port map (
    i => b,
    o => c);
  p1 <= c;
  p2 <= c;
  p4 <= c;
  p6 <= c;
  p7 <= c;
  p8 <= c;
  p9 <= c;
  p10 <= c;
  p11 <= c;
  p12 <= c;
  p13 <= c;
  p15 <= c;
  u4 : g2 port map (
    a => p14,
    b => p16,
    x => d);
  tp5 <= d;
  u5 : inv port map (
    i => d,
    o => e);
  tp6 <= e;
  u6 : inv port map (
    i => e,
    o => f);
  p17 <= f;
  p18 <= f;
  p19 <= f;
  p20 <= f;
  p21 <= f;
  p22 <= f;
  p23 <= f;
  p24 <= f;
  p25 <= f;
  p26 <= f;
  p27 <= f;
  p28 <= f;

end gates;


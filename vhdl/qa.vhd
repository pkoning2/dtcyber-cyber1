-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QA module, rev C
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qaslice is
  
  port (
    d1, clk1, d2, clk2       : in  std_logic;
    tp1, tp3                 : out std_logic;  -- test points
    qb11, qb12, q1, q12, qb2 : out std_logic);

end qaslice;

architecture gates of qaslice is
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
  component latch
    port (
      d, clk : in  std_logic;                 -- data (set), clock
      r      : in  std_logic := '0';          -- optional reset
      q, qb  : out std_logic);                -- q and q.bar
  end component;
  signal t1, t2, t3 : std_logic;
begin  -- gates

  u1 : latch port map (
    d   => d1,
    clk => clk1,
    q   => t1);
  tp1 <= t1;
  u2 : inv2 port map (
    a  => t1,
    y  => t2,
    y2 => q1);
  qb11 <= t2;
  qb12 <= t2;
  u3 : latch port map (
    d   => d2,
    clk => clk2,
    q   => t3);
  tp3 <= t3;
  u4 : g2 port map (
    a => t1,
    b => t3,
    y => q12);
  u5 : inv port map (
    a => t3,
    y => qb2);
  
end gates;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qa is
  
  port (
    p14, p19                     : in  std_logic;
    p11, p15, p12, p16, p27, p26 : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p5, p3, p6, p9, p1           : out std_logic;
    p17, p13, p2, p10, p8        : out std_logic;
    p25, p21, p22, p23, p28      : out std_logic);

end qa;

architecture gates of qa is
  component qaslice
    port (
      d1, clk1, d2, clk2       : in  std_logic;
      tp1, tp3                 : out std_logic;  -- test points
      qb11, qb12, q1, q12, qb2 : out std_logic);
  end component;
begin  -- gates

  u1 : qaslice port map (
    d1   => p11,
    clk1 => p19,
    d2   => p15,
    clk2 => p19,
    tp1  => tp1,
    tp3  => tp3,
    qb11 => p5,
    qb12 => p3,
    q1   => p6,
    q12  => p9,
    qb2  => p1);
  u2 : qaslice port map (
    d1   => p12,
    clk1 => p19,
    d2   => p16,
    clk2 => p14,
    tp1  => tp2,
    tp3  => tp4,
    qb11 => p17,
    qb12 => p13,
    q1   => p2,
    q12  => p10,
    qb2  => p8);
  u3 : qaslice port map (
    d1   => p27,
    clk1 => p14,
    d2   => p26,
    clk2 => p14,
    tp1  => tp5,
    tp3  => tp6,
    qb11 => p25,
    qb12 => p21,
    q1   => p22,
    q12  => p23,
    qb2  => p28);
end gates;

-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PD module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

entity pdslice is
  
  port (
    i : in  std_logic;                        -- input
    clk : in std_logic;                       -- clock
    tp : out std_logic;                       -- test point output
    q, qb : out std_logic);                   -- outputs

end pdslice;

architecture gates of pdslice is
  component inv2
    port (
      a : in  std_logic;                      -- input
      y, y2 : out std_logic);                     -- output
  end component;
  component latch
    port (
      d, clk : in  std_logic;                   -- data (set), clock
      q, qb  : out std_logic);                  -- q and q.bar
  end component;
  signal ti : std_logic;                      -- copy of test point
  signal t2 : std_logic;                      -- buffered output
begin  -- gates

  u1 : latch port map (
    d => i,
    clk => clk,
    q => ti);
  tp <= ti;
  u2 : inv2 port map (
    a => ti,
    y => t2,
    y2 => q);
  qb <= t2;

end gates;

library IEEE;
use IEEE.std_logic_1164.all;

entity pd is
  
  port (
    p16 : in  std_logic;                       -- clock input
    p14, p13, p18, p17 : in  std_logic;  -- inputs
    tp1, tp2, tp5, tp6 : out std_logic;       -- test points
    p11, p7, p9, p6, p4 : out std_logic;  -- bit 1 outputs
    p12, p10, p8, p5, p3 : out std_logic;  -- bit 2 outputs
    p19, p20, p25, p23, p28 : out std_logic;  -- bit 3 outputs
    p22, p24, p26, p21, p27 : out std_logic);  -- bit 4 outputs
end pd;

architecture gates of pd is
  component pdslice
    port (
      i : in  std_logic;                      -- input
      clk : in std_logic;                     -- clock
      tp    : out std_logic;                  -- test point output
      q, qb : out std_logic);                 -- outputs
  end component;
  signal q1, q2, q3, q4 : std_logic;
  signal qb1, qb2, qb3, qb4 : std_logic;
begin  -- gates
  
  u1 : pdslice port map (
    i  => p14,
    clk => p16,
    tp => tp1,
    q  => q1,
    qb => qb1);
  p11 <= q1;
  p7 <= q1;
  p9 <= q1;
  p6 <= qb1;
  p4 <= qb1;
  u2 : pdslice port map (
    i   => p13,
    clk => p16,
    tp  => tp2,
    q   => q2,
    qb  => qb2);
  p12 <= q2;
  p10 <= q2;
  p8 <= q2;
  p5 <= qb2;
  p3 <= qb2;
  u3 : pdslice port map (
    i   => p18,
    clk => p16,
    tp  => tp5,
    q   => q3,
    qb  => qb3);
  p19 <= q3;
  p20 <= q3;
  p25 <= q3;
  p23 <= qb3;
  p28 <= qb3;
  u4 : pdslice port map (
    i   => p17,
    clk => p16,
    tp  => tp6,
    q   => q4,
    qb  => qb4);
  p22 <= q4;
  p24 <= q4;
  p26 <= q4;
  p21 <= qb4;
  p27 <= qb4;
         
end gates;


-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PA module rev C -- memory address register (S register)
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity paslice is
  
  port (
    d, clk : in  std_logic;             -- data and clock
    tp     : out std_logic;             -- test point
    q1, q2 : out std_logic);            -- outputs

end paslice;

architecture gates of paslice is
  component inv2
    port (
      a  : in  std_logic;                     -- input
      y, y2  : out std_logic);                    -- output
  end component;
  component latch
    port (
      d, clk : in  std_logic;                 -- data (set), clock
      r      : in  std_logic := '0';          -- optional reset
      q, qb  : out std_logic);                -- q and q.bar
  end component;
  signal qi, qi2 : std_logic;                 -- latch output
begin  -- gates
  u1 : latch port map (
    d   => d,
    clk => clk,
    q   => qi);
  tp <= qi;
  u2 : inv2 port map (
    a => qi,
    y2 => qi2);
  q1 <= qi2;
  q2 <= qi2;
end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pa is
  
  port (
    p8, p5, p12, p21, p24, p23   : in  std_logic;   -- inputs
    p9, p7, p20, p22             : in  std_logic;   -- strobes
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;   -- test points
    p1, p3, p2, p6, p14, p10     : out std_logic;   -- outputs
    p19, p17, p27, p25, p28, p26 : out std_logic);  -- more outputs

end pa;

architecture gates of pa is
  component paslice
    port (
      d, clk : in  std_logic;           -- data and clock
      tp     : out std_logic;           -- test point
      q1, q2 : out std_logic);          -- outputs
  end component;
  signal b, d : std_logic;       -- strobes
begin  -- gates
  b <= p9 and p7;                       -- gate delay is modeled in latch
  d <= p20 and p22;
  u5 : paslice port map (
    d   => p8,
    clk => b,
    tp  => tp1,
    q1  => p1,
    q2  => p3);
  u6 : paslice port map (
    d   => p5,
    clk => b,
    tp  => tp2,
    q1  => p2,
    q2  => p6);
  u7 : paslice port map (
    d   => p12,
    clk => b,
    tp  => tp3,
    q1  => p14,
    q2  => p10);
  u8 : paslice port map (
    d   => p21,
    clk => d,
    tp  => tp4,
    q1  => p19,
    q2  => p17);
  u9 : paslice port map (
    d   => p24,
    clk => d,
    tp  => tp5,
    q1  => p27,
    q2  => p25);
  u10 : paslice port map (
    d   => p23,
    clk => d,
    tp  => tp6,
    q1  => p28,
    q2  => p26);

end gates;

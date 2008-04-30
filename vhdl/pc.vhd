-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PC module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pcslice is
  
  port (
    d1, d2 : in  std_logic;             -- inputs
    e1, e2 : in  std_logic;             -- enables
    clk    : in  std_logic;             -- clock
    q      : out std_logic);            -- output

end pcslice;

architecture gates of pcslice is
  component latch
    port (
      d, clk : in  std_logic;                 -- data (set), clock
      d2, d3, d4  : in  std_logic := '1';     -- extra data if needed
      q, qb  : out std_logic);                -- q and q.bar
  end component;
  signal d : std_logic := '0';          -- combined input
begin  -- gates
  d <= (d1 and e1) or (d2 and e2);
  u1 : latch port map (
    d   => d,
    clk => clk,
    q   => q);
end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pc is
  
  port (
    p7, p5, p8, p6, p23, p25, p24, p26 : in  std_logic;   -- inputs
    p22, p11                           : in  std_logic;   -- clocks
    p13, p20                           : in  std_logic;   -- selects
    tp1, tp2, tp5, tp6                 : out std_logic;   -- test points
    p14, p9, p18, p21                  : out std_logic;   -- outputs
    p2, p4, p15, p17                   : out std_logic);  -- pullups

end pc;

architecture gates of pc is
  component inv
    port (
      a  : in  std_logic;                     -- input
      y  : out std_logic);                    -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2    : out std_logic);                  -- output
  end component;
  component pcslice
    port (
      d1, d2 : in  std_logic;             -- inputs
      e1, e2 : in  std_logic;             -- enables
      clk    : in  std_logic;             -- clock
      q      : out std_logic);            -- output
  end component;
  signal e, f : std_logic := '0';       -- selects
  signal q1, q2, q3, q4 : std_logic;    -- outputs
begin  -- gates
  p2 <= '0';
  p4 <= '0';
  p15 <= '0';
  p17 <= '0';
  u1 : inv port map (
    a => p13,
    y => e);
  u2 : g2 port map (
    a => e,
    b => p20,
    y => f);
  u3 : pcslice port map (
    d1  => p7,
    e1  => e,
    d2  => p5,
    e2  => f,
    clk => p11,
    q   => q1);
  tp1 <= q1;
  p14 <= q1;
  u4 : pcslice port map (
    d1  => p8,
    e1  => e,
    d2  => p6,
    e2  => f,
    clk => p22,
    q   => q2);
  tp2 <= q2;
  p9 <= q2;
  u5 : pcslice port map (
    d1  => p23,
    e1  => e,
    d2  => p25,
    e2  => f,
    clk => p11,
    q   => q3);
  tp5 <= q3;
  p18 <= q3;
  u6 : pcslice port map (
    d1  => p24,
    e1  => e,
    d2  => p26,
    e2  => f,
    clk => p22,
    q   => q4);
  tp6 <= q4;
  p21 <= q4;
end gates;

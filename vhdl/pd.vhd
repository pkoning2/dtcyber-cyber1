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

entity pdslice is
  
  port (
    i : in  std_logic;                        -- input
    clk : in std_logic;                       -- clock
    tp : out std_logic;                       -- test point output
    q, qb : out std_logic);                   -- outputs

end pdslice;

architecture gates of pdslice is
  component inv
    port (
      i : in  std_logic;                      -- input
      o : out std_logic);                     -- output
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
  u2 : inv port map (
    i => ti,
    o => t2);
  qb <= t2;
  u3 : inv port map (
    i => t2,
    o => q);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;

entity pd is
  
  port (
    p16 : in  std_logic := '1';                       -- clock input
    p14 : in  std_logic := '1';                       -- bit 1 input
    tp1 : out std_logic;                      -- test point 1
    p4  : out std_logic;                      -- bit 1 output, complemented
    p7  : out std_logic;                      -- bit 1 output
    p11 : out std_logic);                     -- bit 1 output

end pd;

architecture gates of pd is
  component pdslice
    port (
      i : in  std_logic;                      -- input
      clk : in std_logic;                     -- clock
      tp    : out std_logic;                  -- test point output
      q, qb : out std_logic);                 -- outputs
  end component;
  signal q : std_logic;
begin  -- gates
  
  u1 : pdslice port map (
    i  => p14,
    clk => p16,
    tp => tp1,
    q  => q,
    qb => p4);
  p7 <= q;
  p11 <= q;
end gates;


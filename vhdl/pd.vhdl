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
    tp : out std_logic;                       -- test point output
    q, qb : out std_logic);                   -- outputs

end pdslice;

architecture gates of pdslice is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      x    : out std_logic);                  -- output
  end component;
  component inv
    port (
      i : in  std_logic;                      -- input
      o : out std_logic);                     -- output
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- inputs
      q, qb : out std_logic);                 -- outputs
  end component;
  signal a, b : std_logic;                    -- clocks
  signal ti : std_logic;                      -- copy of test point
  signal t2 : std_logic;                      -- buffered output
  signal ad : std_logic;                      -- gated input
begin  -- gates

  u1 : g2 port map (
    a => i,
    b => b,
    x => ad);
  u2 : rsflop port map (
    s => ad,
    r => a,
    q => ti);
  tp <= ti;
  u3 : inv port map (
    i => ti,
    o => t2);
  qb <= t2;
  u4 : inv port map (
    i => t2,
    o => q);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;

entity pd is
  
  port (
    p16 : in std_logic;                       -- clock input
    p14 : in std_logic;                       -- bit 1 input
    tp1 : out std_logic;                      -- test point 1
    p4  : out std_logic;                      -- bit 1 output, complemented
    p11 : out std_logic);                     -- bit 1 output

end pd;

architecture gates of pd is
  component pdslice
    port (
      i : in  std_logic;                      -- input
      tp    : out std_logic;                  -- test point output
      q, qb : out std_logic);                 -- outputs
  end component;
  component clearset
    port (
      clk  : in  std_logic;                   -- clock input
      a, b : out std_logic);                  -- clear, set outputs
  end component;
  signal a, b : std_logic;                    -- reset and set clocks
begin  -- gates

  u1 : clearset port map (
    clk => p16,
    a   => a,
    b   => b);
  u2 : pdslice port map (
    i  => p14,
    tp => tp1,
    q  => p11,
    qb => p4);

end gates;


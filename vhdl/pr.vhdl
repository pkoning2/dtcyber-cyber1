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

entity prslice is
  
  port (
    d, clk : in  std_logic;                   -- data, clock
    idata  : in  std_logic;                   -- input data coax
    r      : in  std_logic;                   -- reset
    odata  : out std_logic;                   -- output data coax
    qb     : out std_logic);                  -- output (negated)

end prslice;

architecture gates of prslice is
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
  component coax
    port (
      a : in  std_logic;                        -- source
      x : out std_logic);                       -- destination
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- inputs
      q, qb : out std_logic);                 -- outputs
  end component;
  signal s, s2, rb : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => clk,
    b => d,
    x => s);
  s2 <= s or idata;
  u2 : inv port map (
    i => r,
    o => rb);
  u3 : rsflop port map (
    s  => s2,
    r  => rb,
    qb => qb);
  u4 : coax port map (
    a => s,
    x => odata);
  
end gates;

library IEEE;
use IEEE.std_logic_1164.all;

entity pr is
  
  port (
    p19 : in  std_logic;                      -- clock
    p17 : in  std_logic;                      -- bit 0
    p14 : in  std_logic;                      -- clear
    p6  : in  std_logic;                      -- coax data in bit 0
    p5  : out std_logic;                      -- coax data out bit 0
    p8  : out std_logic);                     -- registered data 0

end pr;

architecture gates of pr is
  component prslice
    port (
      d, clk : in  std_logic;                   -- data, clock
      idata  : in  std_logic;                   -- input data coax
      r      : in  std_logic;                   -- reset
      odata  : out std_logic;                   -- output data coax
      qb     : out std_logic);                  -- output (negated)
  end component;
  component inv
    port (
      i : in  std_logic;                      -- input
      o : out std_logic);                     -- output
  end component;
  signal a, b : std_logic;                    -- buffered clock
begin  -- gates

  u1 : inv port map (
    i => p19,
    o => a);
  u2 : inv port map (
    i => a,
    o => b);
  u3 : prslice port map (
    clk   => b,
    d     => p17,
    r     => p14,
    idata => p6,
    odata => p5,
    qb    => p8);

end gates;


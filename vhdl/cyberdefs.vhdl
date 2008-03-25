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

-- Common definitions for CDC 6600 model

-- signal definitions

package sigs is

  constant v0 : std_logic := '1';             -- low voltage (logic 1)
  constant v1 : std_logic := '0';             -- high voltage (logic 0)
  constant t : time := 5 ns;            -- basic stage delay
  constant tp : time := 10 ns;          -- twisted pair wire delay
  constant tc : time := 25 ns;          -- coax delay (including transistors)
  type testvector is array (natural range <>) of std_logic;  -- test vector from harness
end sigs;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity inv is
  
  port (
    a  : in  std_logic;                       -- input
    x  : out std_logic);                      -- output

end inv;

architecture bool of inv is

begin  -- bool

  x <= a after t;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g2 is
  
  port (
    a, b : in  std_logic;                     -- inputs
    x    : out std_logic);                    -- output

end g2;

architecture bool of g2 is

begin  -- bool

  x <= not (a) or not (b) after t;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g3 is
  
  port (
    a, b, c : in  std_logic;                  -- inputs
    x    : out std_logic);                    -- output

end g3;

architecture bool of g3 is

begin  -- bool

  x <= not (a) or not (b) or not (c) after t;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g4 is
  
  port (
    a, b, c, d : in  std_logic;               -- inputs
    x    : out std_logic);                    -- output

end g4;

architecture bool of g4 is

begin  -- bool

  x <= not (a) or not (b) or not (c) or not (d) after t;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g5 is
  
  port (
    a, b, c, d, e : in  std_logic;            -- inputs
    x    : out std_logic);                    -- output

end g5;

architecture bool of g5 is

begin  -- bool

  x <= not (a) or not (b) or not (c) or not (d) or not (e) after t;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity coax is
  
  port (
    a : in  std_logic;                        -- source
    x : out std_logic);                       -- destination

end coax;

architecture bool of coax is

begin  -- bool

  x <= transport a after tc;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity rsflop is
  port (
    s, r  : in  std_logic;                    -- set, reset
    q, qb : out std_logic);                   -- q and q.bar

end rsflop;

architecture gates of rsflop is
  component g2 is
    port (
      a, b : in  std_logic;                     -- inputs
      x    : out std_logic);                    -- output
  end component;
  signal qi : std_logic;                -- internal copies of the outputs
  signal qib : std_logic;               -- internal copies of the outputs

begin  -- gates
  u1 : g2 port map (
    a => s,
    b => qib,
    x => qi);
  u2 : g2 port map (
    a => qi,
    b => r,
    x => qib);
  q <= qi;
  qb <= qib;
  
end gates;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity clearset is
  
  port (
    clk  : in  std_logic;                     -- clock input
    a, b : out std_logic);                    -- clear and set outputs

end clearset;

architecture gates of clearset is
  signal ai : std_logic;                      -- internal copy of a output
begin  -- gates

  ai <= not (clk) after t;
  a <= ai;
  b <= not (ai) after t;

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity latch is
  
  port (
    d, clk : in  std_logic;                   -- data (set), clock
    q, qb  : out std_logic);                  -- q and q.bar

end latch;

architecture gates of latch is
  signal a, b, ad : std_logic;                -- delayed clks
  component g2 is
    port (
      a, b : in  std_logic;                     -- inputs
      x    : out std_logic);                    -- output
  end component;
  component clearset
    port (
      clk  : in  std_logic;                   -- clock input
      a, b : out std_logic);                  -- clear, set outputs
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- set, reset
      q, qb : out std_logic);                 -- q and q.bar
  end component;
begin  -- gates

  u1 : clearset port map (
    clk => clk,
    a   => a,
    b   => b);
  u2 : g2 port map (
    a => d,
    b => b,
    x => ad);
  u3 : rsflop port map (
    s  => ad,
    r  => a,
    q  => q,
    qb => qb);

end gates;

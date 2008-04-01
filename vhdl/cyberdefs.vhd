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
  type coaxsigs is array (1 to 19) of std_logic;  -- CDC standard coax cable
end sigs;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity inv is
  
  port (
    i  : in  std_logic;                       -- input
    o  : out std_logic);                      -- output

end inv;

architecture bool of inv is
  signal ii : std_logic;
  signal oi : std_logic := '0';
begin  -- bool
  ii <= '1' when i = 'U' else i;
  oi <= not (ii) after t;
  o <= oi;

end bool;

-- The CDC docs talk about the gates as "NOR", meaning "NOT OR".
-- In standard terminology, that's "NAND" (AND NOT).  So we'll
-- write the that way here.

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g2 is
  
  port (
    a, b : in  std_logic;                     -- inputs
    x    : out std_logic);                    -- output

end g2;

architecture bool of g2 is
  signal ai, bi : std_logic;
  signal xi : std_logic := '0';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  xi <= not (ai and bi) after t;
  x <= xi;
  
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
  signal ai, bi, ci : std_logic;
  signal xi : std_logic := '0';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  xi <= not (ai and bi and ci) after t;
  x <= xi;

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
  signal ai, bi, ci, di : std_logic;
  signal xi : std_logic := '0';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  di <= '1' when d = 'U' else d;
  xi <= not (ai and bi and ci and di) after t;
  x <= xi;

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
  signal ai, bi, ci, di, ei : std_logic;
  signal xi : std_logic := '0';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  di <= '1' when d = 'U' else d;
  ei <= '1' when e = 'U' else e;
  xi <= not (ai and bi and ci and di and ei) after t;
  x <= xi;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity cxdriver is
  
  port (
    a : in  std_logic;                        -- source
    x : out std_logic);                       -- destination

end cxdriver;

architecture bool of cxdriver is
  signal ai : std_logic;
  signal xi : std_logic := '1';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  xi <= a after tc;
  x <= xi;
end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity rsflop is
  port (
    s, r  : in  std_logic;                    -- set, reset
    s2, s3, s4, r2, r3, r4  : in  std_logic := '1';  -- extra set, reset if needed
    q, qb : out std_logic);                   -- q and q.bar

end rsflop;

architecture gates of rsflop is
  component g5 is
    port (
      a, b, c, d, e : in  std_logic;            -- inputs
      x    : out std_logic);                    -- output
  end component;
  signal qi : std_logic := '0';                -- internal copies of the outputs
  signal qib : std_logic := '1';               -- internal copies of the outputs
  signal q2, qb2 : std_logic;
begin  -- gates
  u1 : g5 port map (
    a => s,
    b => qib,
    c => s2,
    d => s3,
    e => s4,
    x => qi);
  u2 : g5 port map (
    a => qi,
    b => r,
    c => r2,
    d => r3,
    e => r4,
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
  signal ai : std_logic := '0';               -- internal copy of output
  signal bi : std_logic := '1';               -- internal copy of output
  signal clki : std_logic;
begin  -- gates
  clki <= '1' when clk = 'U' else clk;  
  ai <= not (clki) after t;
  a <= ai;
  bi <= not (ai) after t;
  b <= bi;
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

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

entity tdslice is
  
  port (
    a, b : in  std_logic;                     -- inputs
    tp   : out std_logic;                     -- test point
    x    : out std_logic);                    -- output

end tdslice;

architecture gates of tdslice is
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
  signal ti : std_logic;                      -- internal value of test point
begin  -- gates

  u1 : g2 port map (
    a => a,
    b => b,
    x => ti);
  u2 : inv port map (
    i => ti,
    o => x);
  tp <= ti;
end gates;

library IEEE;
use IEEE.std_logic_1164.all;

entity td is
  
  port (
    p1, p3 : in  std_logic;                   -- inputs
    tp1    : out std_logic;                   -- test point 1
    p6     : out std_logic);                  -- output

end td;

architecture gates of td is
  component tdslice is
    port (
      a, b : in  std_logic;                     -- inputs
      tp   : out std_logic;                     -- test point
      x    : out std_logic);                    -- output
  end component;

begin  -- gates

  u1 : tdslice port map (
    a  => p1,
    b  => p3,
    tp => tp1,
    x  => p6);

end gates;



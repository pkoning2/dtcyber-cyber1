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

entity thslice is
  
  port (
    i  : in  std_logic;                       -- input
    tp : out std_logic;                       -- test point
    o  : out std_logic);                      -- output

end thslice;

architecture gates of thslice is
  component inv
    port (
      i : in  std_logic;                      -- input
      o : out std_logic);                     -- output
  end component;
  signal ti : std_logic;                      -- intermediate value
begin  -- gates

  u1 : inv port map (
    i => i,
    o => ti);
  tp <= ti;
  u2 : inv port map (
    i => ti,
    o => o);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;

entity th is
  
  port (
    p6  : in  std_logic;                      -- bit 1 input
    tp2 : out std_logic;                      -- test point
    p5  : out std_logic);                     -- bit 1 output

end th;

architecture gates of th is
  component thslice
    port (
      i  : in  std_logic;                       -- input
      tp : out std_logic;                       -- test point
      o  : out std_logic);                      -- output
  end component;
begin  -- gates

  u1 : thslice port map (
    i  => p6,
    tp => tp2,
    o  => p5);

end gates;


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

entity pk is
  
  port (
    p19 : in  std_logic;                      -- select channel
    p15 : in  std_logic;                      -- coax full input
    p6  : in  std_logic;                      -- full
    p17 : in  std_logic;                      -- empty
    p8  : in  std_logic;                      -- coax empty input
    p23 : in  std_logic;                      -- master clear
    p5  : in  std_logic;                      -- function
    p3  : in  std_logic;                      -- function coax output
    p9  : out std_logic;                      -- empty coax output
    p10 : out std_logic;                      -- empty out
    p18 : out std_logic;                      -- inactive coax out
    p14 : in  std_logic;                      -- inactive coax in
    p21 : out std_logic;                      -- inactive out
    p25 : in  std_logic;                      -- active coax in
    p22 : out std_logic;                      -- active coax out
    p12 : in  std_logic;                      -- active in
    p27 : in  std_logic;                      -- inactive in
    p24 : out std_logic;                      -- empty out
    p13 : out std_logic;                      -- coax full out
    tp1 : out std_logic;                      -- test point 1
    tp2 : out std_logic;                      -- test point 2
    tp3 : out std_logic;                      -- test point 3
    tp4 : out std_logic;                      -- test point 4
    tp5 : out std_logic;                      -- test point 5
    tp6 : out std_logic);                     -- test point 6

end pk;

architecture gates of pk is
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
  signal b, c : std_logic;                    -- internal enables
  signal ai, ii, fi, ei, fui : std_logic;     -- internal gate output terms
begin  -- gates

  

end gates;

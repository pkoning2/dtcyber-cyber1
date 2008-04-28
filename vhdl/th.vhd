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
    o1, o2, o3, o4  : out std_logic);         -- outputs

end thslice;

architecture gates of thslice is
  component inv
    port (
      a : in  std_logic;                      -- input
      y : out std_logic);                     -- output
  end component;
  signal ti : std_logic;                      -- intermediate value
  signal o : std_logic;
begin  -- gates

  u1 : inv port map (
    a => i,
    y => ti);
  tp <= ti;
  u2 : inv port map (
    a => ti,
    y => o);
  o1 <= o;
  o2 <= o;
  o3 <= o;
  o4 <= o;
  
end gates;

library IEEE;
use IEEE.std_logic_1164.all;

entity th is
  
  port (
    p1, p3, p6, p25, p26, p28  : in  std_logic := '1';  -- inputs
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test point
    p7, p8, p14, p2, p4, p10, p12: out std_logic;  -- outputs
    p16, p18, p20, p24, p5, p9, p11, p13 : out std_logic;  -- outputs
    p17, p19, p23, p27, p15, p21, p22 : out std_logic);  -- outputs

end th;

architecture gates of th is
  component thslice
    port (
      i  : in  std_logic;                       -- input
      tp : out std_logic;                       -- test point
      o1, o2, o3, o4  : out std_logic);         -- outputs
  end component;
begin  -- gates

  u1 : thslice port map (
    i  => p1,
    tp => tp1,
    o1 => p14,
    o2 => p8,
    o3 => p7);
  u2 : thslice port map (
    i  => p6,
    tp => tp2,
    o1 => p5,
    o2 => p9,
    o3 => p11,
    o4 => p13);
  u3 : thslice port map (
    i  => p3,
    tp => tp3,
    o1 => p2,
    o2 => p12,
    o3 => p10,
    o4 => p4);
  u4 : thslice port map (
    i  => p26,
    tp => tp4,
    o1 => p23,
    o2 => p19,
    o3 => p17,
    o4 => p27);
  u5 : thslice port map (
    i  => p25,
    tp => tp5,
    o1 => p16,
    o2 => p18,
    o3 => p20,
    o4 => p24);
  u6 : thslice port map (
    i  => p28,
    tp => tp6,
    o1 => p22,
    o2 => p21,
    o3 => p15);
end gates;


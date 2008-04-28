-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PM module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pm is
  
  port (
    p8, p6, p10, p4 : in std_logic;
    p17, p13, p23, p21, p3, p5, p25 : in std_logic;
    p26, p24, p22 : in std_logic;
    tp1, tp2, tp5, tp6 : out std_logic;  -- test points
    p1, p2, p7, p11, p16, p9 : out std_logic;
    p18, p15, p28, p27 : out std_logic);
  
end pm;

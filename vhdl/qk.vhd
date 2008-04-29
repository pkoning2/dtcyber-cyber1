-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QK module -- incrementer
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qk is
  
  port (
    p13, p14, p18                : in  std_logic;
    p26, p25, p28, p27           : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p9, p24, p20, p3, p5, p7     : out std_logic);

end qk;

-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PS module -- memory sense amplifier
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ps is
  
  port (
    p2, p4, p8, p10, p16, p18, p22, p24, p27 : in    std_logic;
    tp1, tp2, tp3, tp4, tp6                  : out   std_logic;  -- test points
    p5, p11, p19, p25                        : out   std_logic;
    p28, p26                                 : inout misc);      -- bias

end ps;

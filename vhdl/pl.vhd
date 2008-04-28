-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PL module - coax drivers
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pl is
  
  port (
    p9, p13, p20, p11, p18, p22 : in  std_logic;
    p10, p21                    : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p8, p4, p25, p6, p27, p23   : out coaxsig);

end pl;

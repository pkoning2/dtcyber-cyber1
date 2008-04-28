-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PV module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pv is
  
  port (
    p27, p25, p14, p12, p5, p10  : in    std_logic;
    p6, p2, p4, p8               : in    std_logic;
    tp1, tp2, tp6                : out   std_logic;  -- test points
    p1, p13, p28, p7, p11, p19   : out   std_logic;
    p16, p18, p20, p22, p24, p26 : inout misc);

end pv;

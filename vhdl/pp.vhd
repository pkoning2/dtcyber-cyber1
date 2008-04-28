-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PP module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pp is
  
  port (
    p16, p21, p1, p3, p12, p9, p7, p20             : in  std_logic;
    p2, p6, p8, p11, p23, p27, p28, p15            : in  std_logic;
    tp1, tp2, tp5, tp6 : out std_logic;  -- test points
    p14, p4, p5, p10, p13, p18, p22, p25, p26, p17 : out std_logic);

end pp;

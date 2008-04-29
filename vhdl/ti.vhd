-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TI module -- 3 and 4 input gates
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ti is
  
  port (
    p1, p3, p5, p7, p2, p4, p6, p8, p10, p12, p14         : in  std_logic;
    p15, p17, p19, p21, p23, p25, p27, p22, p24, p26, p28 : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6                          : out std_logic;  -- test points
    p13, p9, p11, p16, p18, p20                           : out std_logic);

end ti;

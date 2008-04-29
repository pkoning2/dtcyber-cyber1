-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QF module -- shift network
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qf is
  
  port (
    p21, p23, p25, p3, p5, p7, p17, p19, p9 : in  std_logic;
    p1, p11, p4, p2, p13, p15, p27, p28     : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6            : out std_logic;  -- test points
    p8, p18, p12, p10, p20                  : out std_logic;
    p6, p16, p14, p24, p26, p22             : out std_logic);

end qf;

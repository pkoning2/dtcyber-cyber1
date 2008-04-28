-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PF module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pf is
  
  port (
    p2, p15, p16, p3, p1                   : in  std_logic;
    p5, p26, p4, p27                       : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p17, p11, p20, p10, p19, p13, p24, p12 : out std_logic;
    p21, p9, p22, p14, p18                 : out std_logic);

end pf;

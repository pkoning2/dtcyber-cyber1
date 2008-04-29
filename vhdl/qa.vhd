-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QA module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qa is
  
  port (
    p14, p19                     : in  std_logic;
    p11, p15, p12, p16, p27, p26 : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p5, p3, p6, p9, p1           : out std_logic;
    p17, p13, p2, p10, p8        : out std_logic;
    p25, p21, p22, p23, p28      : out std_logic);

end qa;

-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TE module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity te is
  
  port (
    p4, p6, p25, p10, p21, p3, p8 : in  std_logic;
    p23, p22, p5, p24             : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p9, p15, p11                  : out std_logic;
    p17, p28, p20                 : out std_logic;
    p18, p19, p12, p1, p13, p14   : out std_logic);

end te;

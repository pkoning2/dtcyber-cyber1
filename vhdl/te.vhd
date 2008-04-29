-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TE module -- hex 3-input mux
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity te is
  
  port (
    p6, p4, p2, p7, p13, p12, p24, p26, p28  : in  std_logic;
    p5, p3, p1, p22, p18, p17, p23, p25, p27 : in  std_logic;
    p14, p15, p16                            : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6             : out std_logic;  -- test points
    p9, p10, p21, p8, p19, p20               : out std_logic);

end te;

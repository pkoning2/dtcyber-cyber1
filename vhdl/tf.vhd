-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TF module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tf is
  
  port (
    p8, p22, p7, p21, p6, p24, p5, p23 : in  std_logic;
    p4, p26, p3, p25, p1, p27, p2, p28 : in  std_logic;
    p9, p10, p20                       : in  std_logic;
    tp1, tp2, tp5, tp6                 : out std_logic;  -- test points
    p13, p11, p19                      : out std_logic);

end tf;

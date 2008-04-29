-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QB module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qb is
  
  port (
    p7, p5, p3, p17, p19, p15    : in  std_logic;
    p11, p9, p13, p21, p25, p27  : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p2, p1, p20, p22, p24        : out std_logic;
    p4, p8, p6                   : out std_logic;
    p14, p16, p28, p12, p18, p26 : out std_logic);

end qb;

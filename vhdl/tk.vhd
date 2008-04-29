-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TK module -- 3 to 8 decode
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tk is
  
  port (
    p26, p18, p9       : in  std_logic;
    tp1, tp2, tp5, tp6 : out std_logic;  -- test points
    p11, p8, p22, p27  : out std_logic;
    p3, p7, p25, p6    : out std_logic);

end tk;

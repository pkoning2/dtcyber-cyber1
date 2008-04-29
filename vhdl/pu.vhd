-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PU module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pu is
  
  port (
    p10, p11, p17, p19, p18           : in  std_logic;
    tp1, tp5, tp6                     : out std_logic;  -- test points
    p9, p8, p1, p4, p3, p6, p7        : out std_logic;
    p14, p16, p15                     : inout misc;  -- analog stuff
    p22, p21, p28, p27, p26, p25, p24 : out std_logic);

end pu;

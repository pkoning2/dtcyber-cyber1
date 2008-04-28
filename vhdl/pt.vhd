-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PT module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pt is
  
  port (
    p1, p3, p5, p27, p25, p23, p7 : in  std_logic;
    p4, p21                       : in  std_logic;
    tp1, tp2, tp5, tp6            : out std_logic;  -- test points
    p9, p11, p17, p19, p15        : out std_logic);

end pt;

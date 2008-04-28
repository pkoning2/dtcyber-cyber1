-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PZ module -- memory inhibit driver
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pz is
  
  port (
    p4, p2, p6, p19, p27, p22    : in  std_logic;
    p17, p9, p16, p10            : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p5, p3, p7, p21, p25, p23    : out std_logic);

end pz;

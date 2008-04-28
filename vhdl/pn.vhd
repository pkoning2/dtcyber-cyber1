-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PN module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pn is
  
  port (
    p2, p3, p1, p4, p28, p25, p27, p26                : in  std_logic;
    p20, p18, p17                                     : in  std_logic;
    tp1, tp2, tp5, tp6 : out std_logic;  -- test points
    p15, p5, p12, p8, p21, p19, p24, p22 : out std_logic);

end pn;

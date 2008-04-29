-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QG module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qg is
  
  port (
    p1, p3, p5, p25, p23         : in  std_logic;
    tp1, tp2, tp5, tp6           : out std_logic;  -- test points
    p13, p12, p10, p11, p9, p7   : out std_logic;
    p16, p18, p20, p15, p17, p27 : out std_logic;
    p22, p26, p28, p8, p4, p2    : out std_logic);

end qg;

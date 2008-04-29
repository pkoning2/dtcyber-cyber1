-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TA module -- 6 input gates
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ta is
  
  port (
    p14, p12, p10, p8, p6, p4    : in  std_logic;
    p13, p11, p9, p7, p5, p3     : in  std_logic;
    p16, p18, p20, p22, p24, p26 : in  std_logic;
    p15, p17, p19, p21, p23, p25 : in  std_logic;
    tp1, tp2, tp5, tp6           : out std_logic;  -- test points
    p2, p1, p28, p27             : out std_logic);

end ta;

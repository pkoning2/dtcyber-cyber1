-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PI module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pi is
  
  port (
    p5, p10, p24, p7, p21, p26  : in  coaxsig;
    p16, p15                    : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p9, p12, p13, p14, p28, p27 : out std_logic;
    p1, p4, p18, p17, p22, p19  : out std_logic);

end pi;

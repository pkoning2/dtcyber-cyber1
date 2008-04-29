-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QD module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qd is
  
  port (
    p11, p4, p9, p6, p20, p27, p22, p25 : in  std_logic;
    p8, p18, p23, p13                   : in  std_logic;
    p7, p3, p5, p24, p26, p28           : in  std_logic;
    p21                                 : in  std_logic;
    tp1, tp2, tp5, tp6                  : out std_logic;  -- test points
    p12, p19, p10, p15, p16             : out std_logic);

end qd;

-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PW module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pw is
  
  port (
    p2, p4, p7, p5, p11, p9, p3           : in  std_logic;
    p22, p19, p17, p20, p26, p16, p10     : in  std_logic;
    tp1, tp5, tp6                         : out std_logic;  -- test points
    p8, p23, p21, p14, p12, p25, p27, p28 : out std_logic);

end pw;

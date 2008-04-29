-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QE module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qe is
  
  port (
    p7, p11, p3, p4, p19              : in  std_logic;
    p23, p25, p12, p20, p24, p6, p10  : in  std_logic;
    tp1, tp2, tp5, tp6                : out std_logic;  -- test points
    p8, p2, p1, p5, p18, p16          : out std_logic;
    p26, p28, p21, p27, p17, p15, p13 : out std_logic;
    p22, p9, p14                      : out std_logic);

end qe;

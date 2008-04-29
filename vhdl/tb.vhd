-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TB module -- 3-6 input gates
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tb is
  
  port (
    p1, p3, p5, p7, p9, p11 : in  std_logic;
    p2, p4, p6, p8, p10     : in  std_logic;
    p21, p23, p25, p27      : in  std_logic;
    p24, p26, p28           : in  std_logic;
    tp1, tp2, tp5, tp5      : out std_logic;  -- test points
    p19, p13, p12, p15, p15 : out std_logic;
    p13, p16, p18, p20, p22 : out std_logic);

end tb;

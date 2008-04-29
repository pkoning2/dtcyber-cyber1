-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QC module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qc is
  
  port (
    p6, p4, p8, p15, p19, p25, p27, p23        : in  std_logic;
    p5, p3, p14, p16, p20, p26, p28            : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6               : out std_logic;  -- test points
    p10, p17, p21, p7, p11, p24, p12, p13, p18 : out std_logic);

end qc;

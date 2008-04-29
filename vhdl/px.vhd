-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PX module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity px is
  
  port (
    p11                          : in  coaxsig;
    p7, p2, p5, p9, p6           : in  std_logic;
    p22, p10, p23, p24, p21, p26 : in  std_logic;
    tp1, tp2, tp5, tp6           : out std_logic;  -- test points
    p13, p28, p18, p17, p20      : out std_logic;
    p15, p14, p16, p3, p1        : out std_logic;
    p27, p25, p19                : out std_logic);

end px;

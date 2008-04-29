-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TJ module - two port mux
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tj is
  
  port (
    p10, p9, p8, p7, p6, p5      : in  std_logic;
    p24, p25, p26, p21, p22, p23 : in  std_logic;
    tp1, tp2, tp5, tp6           : out std_logic;  -- test points
    p12, p14, p4, p11            : out std_logic;
    p27, p20, p19, p17           : out std_logic);

end tj;

-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PQ module -- deadstart panel control
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pq is
  
  port (
    p6, p22, p24, p26, p28, p11, p17, p19  : in  std_logic;
    p1, p5, p21                            : in  coaxsig;
    p23, p25, p3, p4, p2, p16              : in  std_logic;
    tp1, tp2, tp5, tp6                     : out std_logic;  -- test points
    p27, p13, p18, p20, p10, p8            : out std_logic;
    p14, p9, p12, p7, p15                  : out std_logic);

end pq;

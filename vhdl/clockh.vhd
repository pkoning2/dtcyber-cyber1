-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell, Al Kossow
--
-- Based on the original design by Seymour Cray and his team
--
-- Clock source.  This pinout is designed to be a drop-in replacement
-- for the HQ module at 1H02 in the standard 6600 (sn 8 and up) design.
-- 
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity clockh is
  
  port (
    clk1, clk2, clk3, clk4 : in std_logic;  -- clocks from the outside world
    p16, p11, p13, p18, p10 : in std_logic := '0'; -- inputs (ignored)
    p12 : in coaxsig := '0';                -- external coax clock (ignored)
    p15, p17, p19, p20 : in std_logic := '0';  -- inputs (ignored)
    p2, p4, p6, p8, p14 : out std_logic;    -- clock phase 3
    p1, p3, p5, p7 : out std_logic;         -- clock phase 4
    p22, p24, p26, p28 : out std_logic;     -- clock phase 1
    p21, p23, p25, p27 : out std_logic);    -- clock phase 2

end clockh;

architecture beh of clockh is
begin  -- beh

  p2 <= clk3;
  p4 <= clk3;
  p6 <= clk3;
  p8 <= clk3;
  p14 <= clk3;
  p1 <= clk4;
  p3 <= clk4;
  p5 <= clk4;
  p7 <= clk4;
  p22 <= clk1;
  p24 <= clk1;
  p26 <= clk1;
  p28 <= clk1;
  p21 <= clk2;
  p23 <= clk2;
  p25 <= clk2;
  p27 <= clk2;
  
end beh;

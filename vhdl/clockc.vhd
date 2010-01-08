-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
--
-- Clock source.  This pinout is designed to be a drop-in replacement
-- for the HQ module at 2I22 in the standard 6600 (sn 32 and up) design.
-- Also used in 6I22, 7I22, and 8I21.
-- 
-------------------------------------------------------------------------------

use work.sigs.all;

entity clockc is
  
  port (
    clk1, clk2, clk3, clk4 : in logicsig;  -- clocks from the outside world
    p16, p11, p13, p18, p10 : in logicsig := '0'; -- inputs (ignored)
    p12 : in coaxsig := '0';                -- external coax clock (ignored)
    p15, p17, p19, p20 : in logicsig := '0';  -- inputs (ignored)
    p2, p4, p6, p8, p14 : out logicsig;    -- clock 95
    p1, p3, p5, p7 : out logicsig;         -- clock 20
    p22, p24, p26, p28 : out logicsig;     -- clock 45
    p21, p23, p25, p27 : out logicsig);    -- clock 70

end clockc;

architecture beh of clockc is
begin  -- beh

  p2 <= clk4;
  p4 <= clk4;
  p6 <= clk4;
  p8 <= clk4;
  p14 <= clk4;
  p1 <= clk1;
  p3 <= clk1;
  p5 <= clk1;
  p7 <= clk1;
  p22 <= clk2;
  p24 <= clk2;
  p26 <= clk2;
  p28 <= clk2;
  p21 <= clk3;
  p23 <= clk3;
  p25 <= clk3;
  p27 <= clk4;
  
end beh;

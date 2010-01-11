-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
--
-- Clock source.  This pinout is designed to be a drop-in replacement
-- for the HQ module at 5I22 in the standard 6600 (sn 32 and up) design.
-- 
-------------------------------------------------------------------------------

use work.sigs.all;

entity clockb is
  
  port (
    clk1, clk2, clk3, clk4 : in logicsig;  -- clocks from the outside world
    p16, p11, p13, p18, p10 : in logicsig := '0'; -- inputs (ignored)
    p12 : in coaxsig := '0';                -- external coax clock (ignored)
    p15, p17, p19, p20 : in logicsig := '0';  -- inputs (ignored)
    p2, p4, p6, p8, p14 : out logicsig;    -- clock 20
    p1, p3, p5, p7 : out logicsig;         -- clock 95
    p22, p24, p26, p28 : out logicsig;     -- clock 45
    p21, p23, p25, p27 : out logicsig);    -- clock 70

end clockb;

architecture beh of clockb is
begin  -- beh

  p2 <= clk1;
  p4 <= clk1;
  p6 <= clk1;
  p8 <= clk1;
  p14 <= clk1;
  p1 <= clk4;
  p3 <= clk4;
  p5 <= clk4;
  p7 <= clk4;
  p22 <= clk2;
  p24 <= clk2;
  p26 <= clk2;
  p28 <= clk2;
  p21 <= clk3;
  p23 <= clk3;
  p25 <= clk3;
  p27 <= clk3;
  
end beh;

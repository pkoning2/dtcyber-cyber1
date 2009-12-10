-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008 by Paul Koning
--
-- Clock source.  This pinout is designed to be a drop-in replacement
-- for the TD module at 1H02 in the standard 6600 (sn 1-7) design.
-- 
-------------------------------------------------------------------------------

use work.sigs.all;

entity clock is
  
  port (
    clk1, clk2, clk3, clk4 : in logicsig;  -- clocks from the outside world
    p15, p17, p23, p19 : out logicsig;  -- clock phase 1
    p6, p12 : out logicsig;            -- clock phase 2
    p5, p11 : out logicsig;            -- clock phase 3
    p16, p20 : out logicsig;           -- clock phase 4
    p21 : out logicsig);               -- clock squarewave

end clock;

architecture beh of clock is
begin  -- beh
  p15 <= clk1;
  p17 <= clk1;
  p23 <= clk1;
  p19 <= clk1;
  p6 <= clk2;
  p12 <= clk2;
  p5 <= clk3;
  p11 <= clk3;
  p16 <= clk4;
  p20 <= clk4;
  p21 <= (clk1 or clk2);
  
end beh;

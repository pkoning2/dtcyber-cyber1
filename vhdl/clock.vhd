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
    sysclk1 : in logicsig;  -- clocks from the outside world
    sysclk2 : in logicsig;
    sysclk3 : in logicsig;
    sysclk4 : in logicsig;
    p15, p17, p23, p19 : out logicsig;  -- clock phase 1
    p6, p12 : out logicsig;            -- clock phase 2
    p5, p11 : out logicsig;            -- clock phase 3
    p16, p20 : out logicsig;           -- clock phase 4
    p21 : out logicsig);               -- clock squarewave

end clock;

architecture beh of clock is
begin  -- beh
  p15 <= sysclk1;
  p17 <= sysclk1;
  p23 <= sysclk1;
  p19 <= sysclk1;
  p6 <= sysclk2;
  p12 <= sysclk2;
  p5 <= sysclk3;
  p11 <= sysclk3;
  p16 <= sysclk4;
  p20 <= sysclk4;
  p21 <= (sysclk1 or sysclk2);
  
end beh;

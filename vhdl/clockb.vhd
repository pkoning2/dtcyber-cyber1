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
    sysclk1 : in logicsig;  -- clocks from the outside world
    sysclk2 : in logicsig;
    sysclk3 : in logicsig;
    sysclk4 : in logicsig;
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

  p2 <= sysclk1;
  p4 <= sysclk1;
  p6 <= sysclk1;
  p8 <= sysclk1;
  p14 <= sysclk1;
  p1 <= sysclk4;
  p3 <= sysclk4;
  p5 <= sysclk4;
  p7 <= sysclk4;
  p22 <= sysclk2;
  p24 <= sysclk2;
  p26 <= sysclk2;
  p28 <= sysclk2;
  p21 <= sysclk3;
  p23 <= sysclk3;
  p25 <= sysclk3;
  p27 <= sysclk3;
  
end beh;

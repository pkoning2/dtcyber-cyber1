-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008 by Paul Koning
--
-- Clock source.  This pinout is designed to be a drop-in replacement
-- for the HQ module at 1H02 in the standard 6600 (sn 8 and up) design.
-- 
-------------------------------------------------------------------------------

use work.sigs.all;

entity clockh is
  
  port (
    sysclk1 : in logicsig;  -- clocks from the outside world
    sysclk2 : in logicsig;
    sysclk3 : in logicsig;
    sysclk4 : in logicsig;
    p16, p11, p13, p18, p10 : in logicsig := '0'; -- inputs (ignored)
    p12 : in coaxsig := '0';                -- external coax clock (ignored)
    p15, p17, p19, p20 : in logicsig := '0';  -- inputs (ignored)
    p2, p4, p6, p8, p14 : out logicsig;    -- clock phase 3
    p1, p3, p5, p7 : out logicsig;         -- clock phase 4
    p22, p24, p26, p28 : out logicsig;     -- clock phase 1
    p21, p23, p25, p27 : out logicsig);    -- clock phase 2

end clockh;

architecture beh of clockh is
begin  -- beh

  p2 <= sysclk3;
  p4 <= sysclk3;
  p6 <= sysclk3;
  p8 <= sysclk3;
  p14 <= sysclk3;
  p1 <= sysclk4;
  p3 <= sysclk4;
  p5 <= sysclk4;
  p7 <= sysclk4;
  p22 <= sysclk1;
  p24 <= sysclk1;
  p26 <= sysclk1;
  p28 <= sysclk1;
  p21 <= sysclk2;
  p23 <= sysclk2;
  p25 <= sysclk2;
  p27 <= sysclk2;
  
end beh;

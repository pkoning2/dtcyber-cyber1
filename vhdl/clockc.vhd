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
    sysclk1 : in logicsig;  -- clocks from the outside world
    sysclk2 : in logicsig;
    sysclk3 : in logicsig;
    sysclk4 : in logicsig;
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

  p2 <= sysclk4;
  p4 <= sysclk4;
  p6 <= sysclk4;
  p8 <= sysclk4;
  p14 <= sysclk4;
  p1 <= sysclk1;
  p3 <= sysclk1;
  p5 <= sysclk1;
  p7 <= sysclk1;
  p22 <= sysclk2;
  p24 <= sysclk2;
  p26 <= sysclk2;
  p28 <= sysclk2;
  p21 <= sysclk3;
  p23 <= sysclk3;
  p25 <= sysclk3;
  p27 <= sysclk3;
  
end beh;

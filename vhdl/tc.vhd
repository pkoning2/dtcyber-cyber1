-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity tc is
  
  port (
    p3, p5, p14, p16            : in  std_logic := '1';  -- inputs
    tp1, tp5                    : out std_logic;  -- test point 1 (stage 1)
    tp2, tp6                    : out std_logic;  -- test point 2 (stage 2)
    p1, p2, p4, p6, p7, p8      : out std_logic;  -- outputs 1
    p9, p10, p11, p12, p13, p15 : out std_logic;  -- outputs 1
    p17, p18, p19, p20, p21, p22 : out std_logic;  -- outputs 2
    p23, p24, p25, p26, p27, p28 : out std_logic);  -- outputs 2

end tc;

architecture bool of tc is
  signal pi3, pi5, pi14, pi16 : std_logic;
  signal in1, in2 : std_logic;
begin  -- bool
  pi3 <= '1' when p3 = 'U' else p3;
  pi5 <= '1' when p5 = 'U' else p5;
  pi14 <= '1' when p14 = 'U' else p14;
  pi16 <= '1' when p16 = 'U' else p16;

  in1 <= not (pi3 and pi5);
  in2 <= not (pi14 and pi16);
  
  -- We could do this as gates but that's too much trouble to
  -- be worth doing.  Just code it as concurrent assignments...
  tp1 <= in1 after t;
  tp2 <= not (in1) after 2 * t;
  p1 <= in1 after 3 * t;
  p2 <= in1 after 3 * t;
  p4 <= in1 after 3 * t;
  p6 <= in1 after 3 * t;
  p7 <= in1 after 3 * t;
  p8 <= in1 after 3 * t;
  p9 <= in1 after 3 * t;
  p10 <= in1 after 3 * t;
  p11 <= in1 after 3 * t;
  p12 <= in1 after 3 * t;
  p13 <= in1 after 3 * t;
  p15 <= in1 after 3 * t;
  tp5 <= in2 after t;
  tp6 <= not (in2) after 2 * t;
  p17 <= in2 after 3 * t;
  p18 <= in2 after 3 * t;
  p19 <= in2 after 3 * t;
  p20 <= in2 after 3 * t;
  p21 <= in2 after 3 * t;
  p22 <= in2 after 3 * t;
  p23 <= in2 after 3 * t;
  p24 <= in2 after 3 * t;
  p25 <= in2 after 3 * t;
  p26 <= in2 after 3 * t;
  p27 <= in2 after 3 * t;
  p28 <= in2 after 3 * t;

end bool;


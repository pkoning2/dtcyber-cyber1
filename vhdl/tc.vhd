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
    p3                          : in  std_logic;  -- input 1
    tp1                         : out std_logic;  -- test point 1 (stage 1)
    tp2                         : out std_logic;  -- test point 2 (stage 2)
    p1, p2, p4, p6, p7, p8      : out std_logic;  -- outputs 1
    p9, p10, p11, p12, p13, p15 : out std_logic);  -- outputs 1

end tc;

architecture bool of tc is
  signal pi3 : std_logic;
begin  -- bool
  pi3 <= '1' when p3 = 'U' else p3;
  
  -- We could do this as gates but that's too much trouble to
  -- be worth doing.  Just code it as concurrent assignments...
  tp1 <= not (pi3) after t;
  tp2 <= pi3 after 2 * t;
  p1 <= pi3 after 3 * t;
  p2 <= pi3 after 3 * t;
  p4 <= pi3 after 3 * t;
  p6 <= pi3 after 3 * t;
  p7 <= pi3 after 3 * t;
  p8 <= pi3 after 3 * t;
  p9 <= pi3 after 3 * t;
  p10 <= pi3 after 3 * t;
  p11 <= pi3 after 3 * t;
  p12 <= pi3 after 3 * t;
  p13 <= pi3 after 3 * t;
  p15 <= pi3 after 3 * t;

end bool;


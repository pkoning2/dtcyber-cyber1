-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- AF module -- 6612 D-A circuit
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.sigs.all;

entity afslice is
  
  port (
    a, b, c, d, e, f : in  std_logic;
    q                : out analog);

end afslice;

architecture beh of afslice is

begin  -- beh
  -- purpose: D/A conversion
  -- type   : combinational
  -- inputs : a, b, c, d, e, f
  -- outputs: q
  ad: process (a, b, c, d, e, f)
    variable acc : integer;
  begin  -- process ad
    acc := 0;
    if a = '1' then
      acc := acc + 1;
    end if;
    if b = '1' then
      acc := acc + 1;
    end if;
    if c = '1' then
      acc := acc + 1;
    end if;
    if d = '1' then
      acc := acc + 1;
    end if;
    if e = '1' then
      acc := acc + 1;
    end if;
    if f = '1' then
      acc := acc + 1;
    end if;
    q <= TO_UNSIGNED (acc, 3);
  end process ad;

end beh;

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

entity clock is
  
  port (
    I, II, III, IV : out std_logic);    -- The four clock phases

end clock;

architecture beh of clock is
  signal clkin : std_logic := '0';      -- The external clock input
  signal p1, p2, p3, p4 : std_logic;    -- internal copies of the phases
begin  -- beh

  -- purpose: The external clock waveform
  -- type   : combinational
  -- inputs : 
  -- outputs: clkin
  extclock: process
  begin  -- process extclock
    clkin <= not (clkin);
    wait for 25 ns;
  end process extclock;

  -- purpose: Generate the four clock phases
  -- type   : combinational
  -- inputs : clkin
  -- outputs: p1, p2, p3, p4
  clkgen: process (clkin)
  begin  -- process clkgen
    if p4 = '1' then
      p1 <= '1';
      p2 <= '0';
      p3 <= '0';
      p4 <= '0';
    elsif p1 = '1' then
      p1 <= '0';
      p2 <= '1';
      p3 <= '0';
      p4 <= '0';
    elsif p2 = '1' then
      p1 <= '0';
      p2 <= '0';
      p3 <= '1';
      p4 <= '0';
    else
      p1 <= '0';
      p2 <= '0';
      p3 <= '0';
      p4 <= '1';
    end if;
  end process clkgen;
  I <= p1;
  II <= p2;
  III <= p3;
  IV <= p4;
end beh;

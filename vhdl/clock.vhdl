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
    p1, p2, p3, p4 : out std_logic;         -- The four clock phases
    p11, p12, p13, p14 : out std_logic);    -- Another copy

end clock;

architecture beh of clock is
  signal clkin : std_logic := '0';      -- The external clock input
  signal i, ii, iii, iv : std_logic;    -- internal copies of the phases
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
  -- outputs: i, ii, iii, iv
  clkgen: process (clkin)
  begin  -- process clkgen
    if iv = '1' then
      i <= '1';
      ii <= '0';
      iii <= '0';
      iv <= '0';
    elsif i = '1' then
      i <= '0';
      ii <= '1';
      iii <= '0';
      iv <= '0';
    elsif ii = '1' then
      i <= '0';
      ii <= '0';
      iii <= '1';
      iv <= '0';
    else
      i <= '0';
      ii <= '0';
      iii <= '0';
      iv <= '1';
    end if;
  end process clkgen;
  p1 <= i;
  p2 <= ii;
  p3 <= iii;
  p4 <= iv;
  p11 <= i;
  p12 <= ii;
  p13 <= iii;
  p14 <= iv;
end beh;

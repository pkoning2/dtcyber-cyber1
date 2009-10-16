-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-------------------------------------------------------------------------------

--  A testbench has no ports.

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use std.textio.all;
use work.sigs.all;

entity cdc_tb is
end cdc_tb;

architecture behav of cdc_tb is
   --  Declaration of the component that will be instantiated.
  component cdc6600 
  port (
    c_1w37_in : in  tpcable;
    clk1, clk2, clk3, clk4, reset : in std_logic
);
  end component;
  signal c_1w37_in : tpcable := idletp;  -- Deadstart switches
  signal reset : std_logic := '1';      -- power-up reset
  signal clk1 : std_logic := '1';        -- clock phase 1
  signal clk2, clk3, clk4 : std_logic := '0';  -- clock phase 2-4
  signal zero : std_logic := '0';
  signal one : std_logic := '1';
begin
   --  Component instantiation.
   uut : cdc6600 port map (
      c_1w37_in => c_1w37_in,
      clk1 => clk1,
      clk2 => clk2,
      clk3 => clk3,
      clk4 => clk4,
      reset => reset
      );
   --  This process does the real job.
   -- purpose: Drive reset and the clocks
   -- type   : combinational
   -- inputs : 
   -- outputs: 
   test: process
     variable minor : integer := 0;     -- minor cycles count
   begin  -- process test
     dtmain;
     reset <= '1';
     wait for 25 ns;
     reset <= '0';

     while TRUE loop
       wait for 25 ns;
       if clk1 = '1' then
         clk1 <= '0';
         clk2 <= '1';
       elsif clk2 = '1' then
         clk2 <= '0';
         clk3 <= '1';
       elsif clk3 = '1' then
         clk3 <= '0';
         clk4 <= '1';
       elsif clk4 = '1' then
         clk4 <= '0';
         clk1 <= '1';
       end if;
       minor := minor + 1;
       if minor = 1 then
         c_1w37_in(2) <= '1';
       elsif minor = 50 then
         c_1w37_in(2) <= '0';         
       end if;
     end loop;
   end process test;
end;

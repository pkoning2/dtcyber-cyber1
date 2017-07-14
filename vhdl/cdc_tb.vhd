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
use IEEE.numeric_bit.all;
use std.textio.all;
use work.sigs.all;

entity cdc_tb is
end cdc_tb;

architecture behav of cdc_tb is
   --  Declaration of the component that will be instantiated.
  component cdc6600 
  port (
    c_1w37_in : in  tpcable;
    c_2w15_in : in  tpcable;
    c_2w16_in : in  tpcable;
    c_2w17_in : in  tpcable;
    reset : in  logicsig;
    sysclk : in clocks
    );
  end component;
  component sysclock 
    port (
      sysclk : out clocks);
  end component;

  signal sysclk : clocks;        -- system clocks
  signal c_1w37_in : tpcable := idletp;  -- Deadstart switches
  signal c_2w15_in : tpcable := idletp;  -- Deadstart panel
  signal c_2w16_in : tpcable := idletp;  -- Deadstart panel
  signal c_2w17_in : tpcable := idletp;  -- Deadstart panel
  signal reset : logicsig := '1';      -- power-up reset
  signal zero : logicsig := '0';
  signal one : logicsig := '1';
begin
   --  Component instantiation.
   uut : cdc6600 port map (
      c_1w37_in => c_1w37_in,
      c_2w15_in => c_2w15_in,
      c_2w16_in => c_2w16_in,
      c_2w17_in => c_2w17_in,
      sysclk => sysclk,
      reset => reset
      );
   clk: sysclock port map (sysclk => sysclk);
   --  This process does the real job.
   -- purpose: Drive reset and the clocks
   -- type   : combinational
   -- inputs : 
   -- outputs: 
   test: process
     variable cycle25 : integer := 0;     -- 25 ns cycles count
   begin  -- process test
     dtmain;
     reset <= '1';
     wait for 30 ns;
     reset <= '0';

     while TRUE loop
       wait for 25 ns;
       cycle25 := cycle25 + 1;
       if cycle25 = 4 then
         c_1w37_in(2) <= '1';
       elsif cycle25 = 180 then
         c_1w37_in(2) <= '0';         
       end if;
     end loop;
   end process test;
end;

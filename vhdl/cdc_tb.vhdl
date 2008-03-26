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
use std.textio.all;
use work.sigs.all;

--  A testbench has no ports.

entity cdc_tb is
end cdc_tb;

architecture behav of cdc_tb is
   --  Declaration of the component that will be instantiated.
  component cdc6600 
    port (
      coax1_1_extern_1 : inout coaxsigs);
  end component;
   --  Specifies which entity is bound with the component.
  signal coax1 : coaxsigs;              -- First coax
begin
   --  Component instantiation.
   uut: cdc6600 port map (coax1_1_extern_1 => coax1);

   --  This process does the real job.
   -- purpose: Read the test script and pass it to the UUT
   -- type   : combinational
   -- inputs : 
   -- outputs: 
   test: process
     variable testdata : coaxsigs;         -- One line worth of test data
     variable l : line;
     file vector_file : text is in "./cdc_tb.txt";  -- test vector file
     variable r : real;
     variable g : boolean;
     variable b : character;
     variable i : integer;
     variable d : time;
   begin  -- process test
     while not endfile (vector_file) loop
       readline (vector_file, l);
       read (l, r, good => g);
       next when not g;
       d := r * 25 ns;
       testdata(1 to 19) := coax1;
       for i in testdata'left to testdata'right loop
         read (l, b);
         if b = '0' then
           testdata(i) := '0';
         elsif b = '1' then
           testdata(i) := '1';
         end if;
       end loop;  -- i
       coax1 <= testdata(1 to 19);
       wait for d;
     end loop;
      assert false report "end of test";
      --  Wait forever; this will finish the simulation.
      wait;
   end process test;
end;

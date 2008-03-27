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
      clk : in std_logic;
      coax1_1_extern_1 : inout coaxsigs);
  end component;
   --  Specifies which entity is bound with the component.
  signal coax1 : coaxsigs;              -- First coax
  signal clk : std_logic := '0';        -- clock source
  type testvec is array (1 to 80) of std_logic;
begin
   --  Component instantiation.
   uut: cdc6600 port map (coax1_1_extern_1 => coax1,
                          clk => clk);

   --  This process does the real job.
   -- purpose: Read the test script and pass it to the UUT
   -- type   : combinational
   -- inputs : 
   -- outputs: 
   test: process
     variable testdata : testvec; -- One line worth of test data
     variable l : line;
     file vector_file : text is in "./cdc_tb.txt";  -- test vector file
     variable g : boolean;
     variable b : character;
     variable i : integer;
     variable d : integer;
   begin  -- process test
     while not endfile (vector_file) loop
       readline (vector_file, l);
       read (l, d);                     -- delay in 25 ns units
       read (l, b);                     -- skip the space separator
       for i in testdata'left to testdata'right loop
         read (l, b, good => g);
         exit when not g;
         if b = '0' then
           testdata(i) := '0';
         elsif b = '1' then
           testdata(i) := '1';
         end if;
       end loop;  -- i
       for i in 1 to 19 loop
         coax1(i) <= testdata(i);
       end loop;  -- i
       for i in 1 to d loop
         wait for 25 ns;
         clk <= not (clk);
       end loop;  -- i
     end loop;
      assert false report "end of test";
      --  Wait forever; this will finish the simulation.
      wait;
   end process test;
end;

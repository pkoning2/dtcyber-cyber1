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

library IEEE;
use IEEE.std_logic_1164.all;
use std.textio.all;
use work.sigs.all;

entity cdc_tb is
end cdc_tb;

architecture behav of cdc_tb is
   --  Declaration of the component that will be instantiated.
  component cdc6600 
    port (
      clk1, clk2, clk3, clk4 : in std_logic);
  end component;
  signal clk1 : std_logic := '1';        -- clock phase 1
  signal clk2, clk3, clk4 : std_logic := '0';  -- clock phase 2-4
  type testvec is array (1 to 80) of std_logic;
  signal coax1 : coaxsigs;
begin
   --  Component instantiation.
   uut: cdc6600 port map (clk1 => clk1,
                          clk2 => clk2,
                          clk3 => clk3,
                          clk4 => clk4);
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
     variable ic, oc : coaxsigs;
     variable c1, c2, c3, c4 : integer := 0;
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
         oc(i) := testdata(i);
       end loop;  -- i
       for i in 1 to d loop
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
       end loop;  -- i
     end loop;
     assert false report "end of test";
     --  Wait forever; this will finish the simulation.
     wait;
   end process test;
end;

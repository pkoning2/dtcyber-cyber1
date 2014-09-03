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

entity cdc_tb2 is
end cdc_tb2;

architecture behav of cdc_tb2 is
   --  Declaration of the component that will be instantiated.
  component chassis12
    port (
      c_12w01_in : in  coaxsigs;
      c_12w02_in : in  coaxsigs;
      c_12w06_in : in  coaxsigs;
      c_12w08_in : in  coaxsigs;
      clk1 : in  logicsig;
      clk2 : in  logicsig;
      clk3 : in  logicsig;
      clk4 : in  logicsig;
      c_12w02_out : out coaxsigs;
      c_12w05_out : out coaxsigs;
      c_12w06_903 : out analog;
      c_12w06_904 : out analog;
      c_12w06_905 : out analog;
      c_12w06_906 : out analog;
      c_12w06_out : out coaxsigs;
      c_12w07_out : out coaxsigs;
      c_12w08_903 : out analog;
      c_12w08_904 : out analog;
      c_12w08_905 : out analog;
      c_12w08_906 : out analog;
      c_12w08_out : out coaxsigs);
  end component;
  signal clk1 : logicsig := '1';        -- clock phase 1
  signal clk2, clk3, clk4 : logicsig := '0';  -- clock phase 2-4
  type testvec is array (1 to 80) of logicsig;
  signal coax1 : coaxsigs := ('0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0');
  signal w_12w1 : coaxsigs := ('0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0');
  signal w_12w2 : coaxsigs;
  signal zero : logicsig := '0';
  signal one : logicsig := '1';
  constant idle : coaxsigs := ('0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0');
begin
   --  Component instantiation.
   uut: chassis12 port map (clk1 => clk1,
                          clk2 => clk2,
                          clk3 => clk3,
                          clk4 => clk4,
                          c_12w01_in => w_12w1,
                          c_12w02_in => w_12w2,
                          c_12w06_in => idle,
                          c_12w08_in => idle);
   --  This process does the real job.
   -- purpose: Read the test script and pass it to the UUT
   -- type   : combinational
   -- inputs : 
   -- outputs: 
   test: process
     variable testdata : testvec; -- One line worth of test data
     variable l : line;
     file vector_file : text is in "./cdc_tb2.txt";  -- test vector file
     variable g : boolean;
     variable b : character;
     variable i : integer;
     variable d : integer;
     variable ic, oc : coaxsigs;
     variable c1, c2, c3, c4 : integer := 0;
     variable x, y : integer := 0;
     variable unblank : integer := 0;
     constant space : string := " ";
     constant zero : string := "0";
     constant one : string := "1";
     variable llen : integer;
     variable ten : integer := 9;
   begin  -- process test
     --dtmain;
     while not endfile (vector_file) loop
       readline (vector_file, l);
       read (l, d);                     -- delay in 25 ns units
       read (l, b);                     -- skip the space separator
       for i in testdata'left to testdata'right loop
         read (l, b, good => g);
         exit when not g;
         llen := i;
         if b = '0' then
           testdata(i) := '0';
         elsif b = '1' then
           testdata(i) := '1';
         end if;
       end loop;  -- i
       for i in 1 to 19 loop
         -- coax1(i) <= testdata(i);
         oc(i - 1) := testdata(i);
       end loop;  -- i
       write(l, d);
       write (l, space);
       for i in testdata'left to llen loop
         if testdata(i) = '0' then
           write (l, zero);
         else
           write (l, one);
         end if;
       end loop;  -- i
       writeline (output, l);
       for i in 1 to d loop
         wait for 25 ns;
         if clk2 = '1' then
           w_12w2(17) <= '1';
           ten := ten + 1;
           if ten > 9 then
             ten := 0;
             w_12w2(18) <= '1';
           else
             w_12w2(18) <= '0';
           end if;
         else
           w_12w2(17) <= '0';
           w_12w2(18) <= '0';
         end if;
         if clk3 = '1' then
           w_12w1 <= oc;
           oc := idle;
         else
           w_12w1 <= idle;
         end if;
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

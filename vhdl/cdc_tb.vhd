-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-------------------------------------------------------------------------------

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
      clk1, clk2, clk3, clk4 : in std_logic;
      w02_90 : in coaxsig;
      w02_900 : in coaxsig;
      w02_901 : in coaxsig;
      w02_902 : in coaxsig;
      w02_903 : in coaxsig;
      w02_904 : in coaxsig;
      w02_905 : in coaxsig;
      w02_906 : in coaxsig;
      w02_907 : in coaxsig;
      w02_91 : in coaxsig;
      w02_92 : in coaxsig;
      w02_93 : in coaxsig;
      w02_94 : in coaxsig;
      w02_95 : in coaxsig;
      w02_96 : in coaxsig;
      w02_97 : in coaxsig;
      w02_98 : in coaxsig;
      w02_99 : in coaxsig;
      w03_900 : out coaxsig;
      w03_901 : out coaxsig;
      w03_902 : out coaxsig;
      w03_903 : out coaxsig;
      w03_904 : out coaxsig;
      w03_905 : out coaxsig;
      w03_906 : out coaxsig;
      w03_907 : out coaxsig;
      w03_908 : out coaxsig;
      w03_91 : out coaxsig;
      w03_92 : out coaxsig;
      w03_93 : out coaxsig;
      w03_94 : out coaxsig;
      w03_95 : out coaxsig;
      w03_96 : out coaxsig;
      w03_97 : out coaxsig;
      w03_98 : out coaxsig;
      w03_99 : out coaxsig;
      w04_90 : in coaxsig;
      w04_900 : in coaxsig;
      w04_901 : in coaxsig;
      w04_902 : in coaxsig;
      w04_903 : in coaxsig;
      w04_904 : in coaxsig;
      w04_91 : in coaxsig;
      w04_92 : in coaxsig;
      w04_93 : in coaxsig;
      w04_94 : in coaxsig;
      w04_95 : in coaxsig;
      w04_96 : in coaxsig;
      w04_97 : in coaxsig;
      w04_98 : in coaxsig;
      w04_99 : in coaxsig;
      w05_90 : in coaxsig;
      w05_900 : in coaxsig;
      w05_901 : in coaxsig;
      w05_902 : in coaxsig;
      w05_903 : in coaxsig;
      w05_904 : in coaxsig;
      w05_905 : in coaxsig;
      w05_906 : in coaxsig;
      w05_91 : in coaxsig;
      w05_92 : in coaxsig;
      w05_93 : in coaxsig;
      w05_94 : in coaxsig;
      w05_95 : in coaxsig;
      w05_96 : in coaxsig;
      w05_97 : in coaxsig;
      w05_98 : in coaxsig;
      w05_99 : in coaxsig;
      w061_905 : in coaxsig;
      w06_90 : in coaxsig;
      w06_900 : in coaxsig;
      w06_901 : in coaxsig;
      w06_902 : in coaxsig;
      w06_903 : in coaxsig;
      w06_904 : in coaxsig;
      w06_906 : in coaxsig;
      w06_907 : in coaxsig;
      w06_91 : in coaxsig;
      w06_92 : in coaxsig;
      w06_93 : in coaxsig;
      w06_94 : in coaxsig;
      w06_95 : in coaxsig;
      w06_96 : in coaxsig;
      w07_90 : in coaxsig;
      w07_900 : in coaxsig;
      w07_901 : in coaxsig;
      w07_902 : in coaxsig;
      w07_903 : in coaxsig;
      w07_904 : in coaxsig;
      w07_905 : in coaxsig;
      w07_907 : in coaxsig;
      w07_91 : in coaxsig;
      w07_92 : in coaxsig;
      w07_93 : in coaxsig;
      w07_94 : in coaxsig;
      w07_95 : in coaxsig;
      w08_90 : out coaxsig;
      w08_900 : out coaxsig;
      w08_901 : out coaxsig;
      w08_902 : out coaxsig;
      w08_903 : out coaxsig;
      w08_904 : out coaxsig;
      w08_905 : in coaxsig;
      w08_91 : out coaxsig;
      w08_92 : out coaxsig;
      w08_93 : out coaxsig;
      w08_94 : out coaxsig;
      w08_95 : out coaxsig;
      w08_96 : out coaxsig;
      w08_97 : out coaxsig;
      w08_98 : out coaxsig;
      w08_99 : out coaxsig;
      w09_90 : out coaxsig;
      w09_900 : out coaxsig;
      w09_901 : out coaxsig;
      w09_902 : out coaxsig;
      w09_903 : out coaxsig;
      w09_904 : out coaxsig;
      w09_91 : out coaxsig;
      w09_92 : out coaxsig;
      w09_93 : out coaxsig;
      w09_94 : out coaxsig;
      w09_95 : out coaxsig;
      w09_96 : out coaxsig;
      w09_97 : out coaxsig;
      w09_98 : out coaxsig;
      w09_99 : out coaxsig;
      w10_90 : out coaxsig;
      w10_900 : out coaxsig;
      w10_901 : out coaxsig;
      w10_902 : out coaxsig;
      w10_903 : out coaxsig;
      w10_904 : out coaxsig;
      w10_91 : out coaxsig;
      w10_92 : out coaxsig;
      w10_93 : out coaxsig;
      w10_94 : out coaxsig;
      w10_95 : out coaxsig;
      w10_96 : out coaxsig;
      w10_97 : out coaxsig;
      w10_98 : out coaxsig;
      w10_99 : out coaxsig;
      w11_90 : out coaxsig;
      w11_900 : out coaxsig;
      w11_901 : out coaxsig;
      w11_902 : out coaxsig;
      w11_903 : out coaxsig;
      w11_904 : out coaxsig;
      w11_91 : out coaxsig;
      w11_92 : out coaxsig;
      w11_93 : out coaxsig;
      w11_94 : out coaxsig;
      w11_95 : out coaxsig;
      w11_96 : out coaxsig;
      w11_97 : out coaxsig;
      w11_98 : out coaxsig;
      w11_99 : out coaxsig;
      w12_900 : in coaxsig;
      w12_902 : out coaxsig;
      w12_905 : out coaxsig);
  end component;
  signal clk1 : std_logic := '1';        -- clock phase 1
  signal clk2, clk3, clk4 : std_logic := '0';  -- clock phase 2-4
  type testvec is array (1 to 80) of std_logic;
  signal coax1 : coaxsigs;
  signal   w02_90 : coaxsig := '0';
  signal   w02_900 : coaxsig := '0';
  signal   w02_901 : coaxsig := '0';
  signal   w02_902 : coaxsig := '0';
  signal   w02_903 : coaxsig := '0';
  signal   w02_904 : coaxsig := '0';
  signal   w02_905 : coaxsig := '0';
  signal   w02_906 : coaxsig := '0';
  signal   w02_907 : coaxsig := '0';
  signal   w02_91 : coaxsig := '0';
  signal   w02_92 : coaxsig := '0';
  signal   w02_93 : coaxsig := '0';
  signal   w02_94 : coaxsig := '0';
  signal   w02_95 : coaxsig := '0';
  signal   w02_96 : coaxsig := '0';
  signal   w02_97 : coaxsig := '0';
  signal   w02_98 : coaxsig := '0';
  signal   w02_99 : coaxsig := '0';
  signal   w04_90 : coaxsig := '0';
  signal   w04_900 : coaxsig := '0';
  signal   w04_901 : coaxsig := '0';
  signal   w04_902 : coaxsig := '0';
  signal   w04_903 : coaxsig := '0';
  signal   w04_904 : coaxsig := '0';
  signal   w04_91 : coaxsig := '0';
  signal   w04_92 : coaxsig := '0';
  signal   w04_93 : coaxsig := '0';
  signal   w04_94 : coaxsig := '0';
  signal   w04_95 : coaxsig := '0';
  signal   w04_96 : coaxsig := '0';
  signal   w04_97 : coaxsig := '0';
  signal   w04_98 : coaxsig := '0';
  signal   w04_99 : coaxsig := '0';
  signal   w05_90 : coaxsig := '0';
  signal   w05_900 : coaxsig := '0';
  signal   w05_901 : coaxsig := '0';
  signal   w05_902 : coaxsig := '0';
  signal   w05_903 : coaxsig := '0';
  signal   w05_904 : coaxsig := '0';
  signal   w05_905 : coaxsig := '0';
  signal   w05_906 : coaxsig := '0';
  signal   w05_91 : coaxsig := '0';
  signal   w05_92 : coaxsig := '0';
  signal   w05_93 : coaxsig := '0';
  signal   w05_94 : coaxsig := '0';
  signal   w05_95 : coaxsig := '0';
  signal   w05_96 : coaxsig := '0';
  signal   w05_97 : coaxsig := '0';
  signal   w05_98 : coaxsig := '0';
  signal   w05_99 : coaxsig := '0';
  signal   w061_905 : coaxsig := '0';
  signal   w06_90 : coaxsig := '0';
  signal   w06_900 : coaxsig := '0';
  signal   w06_901 : coaxsig := '0';
  signal   w06_902 : coaxsig := '0';
  signal   w06_903 : coaxsig := '0';
  signal   w06_904 : coaxsig := '0';
  signal   w06_906 : coaxsig := '0';
  signal   w06_907 : coaxsig := '0';
  signal   w06_91 : coaxsig := '0';
  signal   w06_92 : coaxsig := '0';
  signal   w06_93 : coaxsig := '0';
  signal   w06_94 : coaxsig := '0';
  signal   w06_95 : coaxsig := '0';
  signal   w06_96 : coaxsig := '0';
  signal   w07_90 : coaxsig := '0';
  signal   w07_900 : coaxsig := '0';
  signal   w07_901 : coaxsig := '0';
  signal   w07_902 : coaxsig := '0';
  signal   w07_903 : coaxsig := '0';
  signal   w07_904 : coaxsig := '0';
  signal   w07_905 : coaxsig := '0';
  signal   w07_907 : coaxsig := '0';
  signal   w07_91 : coaxsig := '0';
  signal   w07_92 : coaxsig := '0';
  signal   w07_93 : coaxsig := '0';
  signal   w07_94 : coaxsig := '0';
  signal   w07_95 : coaxsig := '0';
  signal   w08_905 :  coaxsig := '0';
  signal   w12_900 :  coaxsig := '0';
begin
   --  Component instantiation.
   uut: cdc6600 port map (clk1 => clk1,
                          clk2 => clk2,
                          clk3 => clk3,
                          clk4 => clk4,
      w02_90 => w02_90,
      w02_900 => w02_900,
      w02_901 => w02_901,
      w02_902 => w02_902,
      w02_903 => w02_903,
      w02_904 => w02_904,
      w02_905 => w02_905,
      w02_906 => w02_906,
      w02_907 => w02_907,
      w02_91 => w02_91,
      w02_92 => w02_92,
      w02_93 => w02_93,
      w02_94 => w02_94,
      w02_95 => w02_95,
      w02_96 => w02_96,
      w02_97 => w02_97,
      w02_98 => w02_98,
      w02_99 => w02_99,
      w04_90 => w04_90,
      w04_900 => w04_900,
      w04_901 => w04_901,
      w04_902 => w04_902,
      w04_903 => w04_903,
      w04_904 => w04_904,
      w04_91 => w04_91,
      w04_92 => w04_92,
      w04_93 => w04_93,
      w04_94 => w04_94,
      w04_95 => w04_95,
      w04_96 => w04_96,
      w04_97 => w04_97,
      w04_98 => w04_98,
      w04_99 => w04_99,
      w05_90 => w05_90,
      w05_900 => w05_900,
      w05_901 => w05_901,
      w05_902 => w05_902,
      w05_903 => w05_903,
      w05_904 => w05_904,
      w05_905 => w05_905,
      w05_906 => w05_906,
      w05_91 => w05_91,
      w05_92 => w05_92,
      w05_93 => w05_93,
      w05_94 => w05_94,
      w05_95 => w05_95,
      w05_96 => w05_96,
      w05_97 => w05_97,
      w05_98 => w05_98,
      w05_99 => w05_99,
      w061_905 => w061_905,
      w06_90 => w06_90,
      w06_900 => w06_900,
      w06_901 => w06_901,
      w06_902 => w06_902,
      w06_903 => w06_903,
      w06_904 => w06_904,
      w06_906 => w06_906,
      w06_907 => w06_907,
      w06_91 => w06_91,
      w06_92 => w06_92,
      w06_93 => w06_93,
      w06_94 => w06_94,
      w06_95 => w06_95,
      w06_96 => w06_96,
      w07_90 => w07_90,
      w07_900 => w07_900,
      w07_901 => w07_901,
      w07_902 => w07_902,
      w07_903 => w07_903,
      w07_904 => w07_904,
      w07_905 => w07_905,
      w07_907 => w07_907,
      w07_91 => w07_91,
      w07_92 => w07_92,
      w07_93 => w07_93,
      w07_94 => w07_94,
      w07_95 => w07_95,
     w08_905 => w08_905,
     w12_900 => w12_900);
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

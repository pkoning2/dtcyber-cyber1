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
use IEEE.numeric_std.all;
use std.textio.all;
use work.sigs.all;

entity cdc_tb is
end cdc_tb;

architecture behav of cdc_tb is
   --  Declaration of the component that will be instantiated.
  component cdc6600 
    port (
      clk1, clk2, clk3, clk4 : in std_logic;
      w_12w1_90 : in coaxsig;
      w_12w1_900 : in coaxsig;
      w_12w1_901 : in coaxsig;
      w_12w1_902 : in coaxsig;
      w_12w1_903 : in coaxsig;
      w_12w1_904 : in coaxsig;
      w_12w1_905 : in coaxsig;
      w_12w1_906 : in coaxsig;
      w_12w1_907 : in coaxsig;
      w_12w1_91 : in coaxsig;
      w_12w1_92 : in coaxsig;
      w_12w1_93 : in coaxsig;
      w_12w1_94 : in coaxsig;
      w_12w1_95 : in coaxsig;
      w_12w1_96 : in coaxsig;
      w_12w1_97 : in coaxsig;
      w_12w1_98 : in coaxsig;
      w_12w1_99 : in coaxsig;
      w_12w2_90 : out coaxsig;
      w_12w2_900 : out coaxsig;
      w_12w2_901 : out coaxsig;
      w_12w2_902 : out coaxsig;
      w_12w2_903 : out coaxsig;
      w_12w2_904 : out coaxsig;
      w_12w2_905 : out coaxsig;
      w_12w2_906 : in coaxsig;
      w_12w2_907 : in coaxsig;
      w_12w2_91 : out coaxsig;
      w_12w2_92 : out coaxsig;
      w_12w2_93 : out coaxsig;
      w_12w2_94 : out coaxsig;
      w_12w2_95 : out coaxsig;
      w_12w2_96 : out coaxsig;
      w_12w2_97 : out coaxsig;
      w_12w2_98 : out coaxsig;
      w_12w2_99 : out coaxsig;
      w_12w5_90 : out std_logic;
      w_12w5_900 : out std_logic;
      w_12w5_901 : out std_logic;
      w_12w5_902 : out std_logic;
      w_12w5_903 : out std_logic;
      w_12w5_904 : out std_logic;
      w_12w5_905 : out std_logic;
      w_12w5_906 : out std_logic;
      w_12w5_907 : out std_logic;
      w_12w5_908 : out std_logic;
      w_12w5_91 : out std_logic;
      w_12w5_92 : out std_logic;
      w_12w5_93 : out std_logic;
      w_12w5_94 : out std_logic;
      w_12w5_95 : out std_logic;
      w_12w5_96 : out std_logic;
      w_12w5_97 : out std_logic;
      w_12w5_98 : out std_logic;
      w_12w5_99 : out std_logic;
      w_12w6_90 : in std_logic;
      w_12w6_900 : out std_logic;
      w_12w6_901 : out std_logic;
      w_12w6_903 : out analog;
      w_12w6_904 : out analog;
      w_12w6_905 : out analog;
      w_12w6_906 : out analog;
      w_12w6_91 : in std_logic;
      w_12w6_92 : in std_logic;
      w_12w6_93 : in std_logic;
      w_12w6_94 : in std_logic;
      w_12w6_95 : in std_logic;
      w_12w6_96 : in std_logic;
      w_12w6_97 : in std_logic;
      w_12w6_98 : out std_logic;
      w_12w6_99 : out std_logic;
      w_12w7_90 : out std_logic;
      w_12w7_900 : out std_logic;
      w_12w7_901 : out std_logic;
      w_12w7_902 : out std_logic;
      w_12w7_903 : out std_logic;
      w_12w7_904 : out std_logic;
      w_12w7_905 : out std_logic;
      w_12w7_906 : out std_logic;
      w_12w7_907 : out std_logic;
      w_12w7_908 : out std_logic;
      w_12w7_91 : out std_logic;
      w_12w7_92 : out std_logic;
      w_12w7_93 : out std_logic;
      w_12w7_94 : out std_logic;
      w_12w7_95 : out std_logic;
      w_12w7_96 : out std_logic;
      w_12w7_97 : out std_logic;
      w_12w7_98 : out std_logic;
      w_12w7_99 : out std_logic;
      w_12w8_90 : in std_logic;
      w_12w8_900 : out std_logic;
      w_12w8_901 : out std_logic;
      w_12w8_903 : out analog;
      w_12w8_904 : out analog;
      w_12w8_905 : out analog;
      w_12w8_908 : out analog;
      w_12w8_91 : in std_logic;
      w_12w8_92 : in std_logic;
      w_12w8_93 : in std_logic;
      w_12w8_94 : in std_logic;
      w_12w8_95 : in std_logic;
      w_12w8_96 : in std_logic;
      w_12w8_97 : in std_logic;
      w_12w8_98 : out std_logic;
      w_12w8_99 : out std_logic);
  end component;
  signal clk1 : std_logic := '1';        -- clock phase 1
  signal clk2, clk3, clk4 : std_logic := '0';  -- clock phase 2-4
  type testvec is array (1 to 80) of std_logic;
  signal coax1 : coaxsigs;
  signal w_12w1_90 :  coaxsig := '0';
  signal w_12w1_900 :  coaxsig := '0';
  signal w_12w1_901 :  coaxsig := '0';
  signal w_12w1_902 :  coaxsig := '0';
  signal w_12w1_903 :  coaxsig := '0';
  signal w_12w1_904 :  coaxsig := '0';
  signal w_12w1_905 :  coaxsig := '0';
  signal w_12w1_906 :  coaxsig := '0';
  signal w_12w1_907 :  coaxsig := '0';
  signal w_12w1_91 :  coaxsig := '0';
  signal w_12w1_92 :  coaxsig := '0';
  signal w_12w1_93 :  coaxsig := '0';
  signal w_12w1_94 :  coaxsig := '0';
  signal w_12w1_95 :  coaxsig := '0';
  signal w_12w1_96 :  coaxsig := '0';
  signal w_12w1_97 :  coaxsig := '0';
  signal w_12w1_98 :  coaxsig := '0';
  signal w_12w1_99 :  coaxsig := '0';
  signal w_12w2_90 :  coaxsig := '0';
  signal w_12w2_900 :  coaxsig;
  signal w_12w2_901 :  coaxsig;
  signal w_12w2_902 :  coaxsig;
  signal w_12w2_903 :  coaxsig;
  signal w_12w2_904 :  coaxsig;
  signal w_12w2_905 :  coaxsig;
  signal w_12w2_906 :  coaxsig := '0';
  signal w_12w2_907 :  coaxsig := '0';
  signal w_12w2_91 :  coaxsig;
  signal w_12w2_92 :  coaxsig;
  signal w_12w2_93 :  coaxsig;
  signal w_12w2_94 :  coaxsig;
  signal w_12w2_95 :  coaxsig;
  signal w_12w2_96 :  coaxsig;
  signal w_12w2_97 :  coaxsig;
  signal w_12w2_98 :  coaxsig;
  signal w_12w2_99 :  coaxsig;
  signal w_12w5_90 :  std_logic;
  signal w_12w5_900 :  std_logic;
  signal w_12w5_901 :  std_logic;
  signal w_12w5_902 :  std_logic;
  signal w_12w5_903 :  std_logic;
  signal w_12w5_904 :  std_logic;
  signal w_12w5_905 :  std_logic;
  signal w_12w5_906 :  std_logic;
  signal w_12w5_907 :  std_logic;
  signal w_12w5_908 :  std_logic;
  signal w_12w5_91 :  std_logic;
  signal w_12w5_92 :  std_logic;
  signal w_12w5_93 :  std_logic;
  signal w_12w5_94 :  std_logic;
  signal w_12w5_95 :  std_logic;
  signal w_12w5_96 :  std_logic;
  signal w_12w5_97 :  std_logic;
  signal w_12w5_98 :  std_logic;
  signal w_12w5_99 :  std_logic;
  signal w_12w6_90 :  std_logic := '0';
  signal w_12w6_900 :  std_logic;
  signal w_12w6_901 :  std_logic;
  signal w_12w6_903 :  analog;
  signal w_12w6_904 :  analog;
  signal w_12w6_905 :  analog;
  signal w_12w6_906 :  analog;
  signal w_12w6_91 :  std_logic := '0';
  signal w_12w6_92 :  std_logic := '0';
  signal w_12w6_93 :  std_logic := '0';
  signal w_12w6_94 :  std_logic := '0';
  signal w_12w6_95 :  std_logic := '0';
  signal w_12w6_96 :  std_logic := '0';
  signal w_12w6_97 :  std_logic := '0';
  signal w_12w6_98 :  std_logic := '0';
  signal w_12w6_99 :  std_logic;
  signal w_12w7_90 :  std_logic;
  signal w_12w7_900 :  std_logic;
  signal w_12w7_901 :  std_logic;
  signal w_12w7_902 :  std_logic;
  signal w_12w7_903 :  std_logic;
  signal w_12w7_904 :  std_logic;
  signal w_12w7_905 :  std_logic;
  signal w_12w7_906 :  std_logic;
  signal w_12w7_907 :  std_logic;
  signal w_12w7_908 :  std_logic;
  signal w_12w7_91 :  std_logic;
  signal w_12w7_92 :  std_logic;
  signal w_12w7_93 :  std_logic;
  signal w_12w7_94 :  std_logic;
  signal w_12w7_95 :  std_logic;
  signal w_12w7_96 :  std_logic;
  signal w_12w7_97 :  std_logic;
  signal w_12w7_98 :  std_logic;
  signal w_12w7_99 :  std_logic;
  signal w_12w8_90 :  std_logic := '0';
  signal w_12w8_900 :  std_logic;
  signal w_12w8_901 :  std_logic;
  signal w_12w8_903 :  analog;
  signal w_12w8_904 :  analog;
  signal w_12w8_905 :  analog;
  signal w_12w8_908 :  analog;
  signal w_12w8_91 :  std_logic := '0';
  signal w_12w8_92 :  std_logic := '0';
  signal w_12w8_93 :  std_logic := '0';
  signal w_12w8_94 :  std_logic := '0';
  signal w_12w8_95 :  std_logic := '0';
  signal w_12w8_96 :  std_logic := '0';
  signal w_12w8_97 :  std_logic := '0';
  signal w_12w8_98 :  std_logic;
  signal w_12w8_99 :  std_logic;
begin
   --  Component instantiation.
   uut: cdc6600 port map (clk1 => clk1,
                          clk2 => clk2,
                          clk3 => clk3,
                          clk4 => clk4,
                          w_12w1_90 => w_12w1_90,
                          w_12w1_900 => w_12w1_900,
                          w_12w1_901 => w_12w1_901,
                          w_12w1_902 => w_12w1_902,
                          w_12w1_903 => w_12w1_903,
                          w_12w1_904 => w_12w1_904,
                          w_12w1_905 => w_12w1_905,
                          w_12w1_906 => w_12w1_906,
                          w_12w1_907 => w_12w1_907,
                          w_12w1_91 => w_12w1_91,
                          w_12w1_92 => w_12w1_92,
                          w_12w1_93 => w_12w1_93,
                          w_12w1_94 => w_12w1_94,
                          w_12w1_95 => w_12w1_95,
                          w_12w1_96 => w_12w1_96,
                          w_12w1_97 => w_12w1_97,
                          w_12w1_98 => w_12w1_98,
                          w_12w1_99 => w_12w1_99,
                          w_12w2_90 => w_12w2_90,
                          w_12w2_900 => w_12w2_900,
                          w_12w2_901 => w_12w2_901,
                          w_12w2_902 => w_12w2_902,
                          w_12w2_903 => w_12w2_903,
                          w_12w2_904 => w_12w2_904,
                          w_12w2_905 => w_12w2_905,
                          w_12w2_906 => w_12w2_906,
                          w_12w2_907 => w_12w2_907,
                          w_12w2_91 => w_12w2_91,
                          w_12w2_92 => w_12w2_92,
                          w_12w2_93 => w_12w2_93,
                          w_12w2_94 => w_12w2_94,
                          w_12w2_95 => w_12w2_95,
                          w_12w2_96 => w_12w2_96,
                          w_12w2_97 => w_12w2_97,
                          w_12w2_98 => w_12w2_98,
                          w_12w2_99 => w_12w2_99,
                          w_12w5_90 => w_12w5_90,
                          w_12w5_900 => w_12w5_900,
                          w_12w5_901 => w_12w5_901,
                          w_12w5_902 => w_12w5_902,
                          w_12w5_903 => w_12w5_903,
                          w_12w5_904 => w_12w5_904,
                          w_12w5_905 => w_12w5_905,
                          w_12w5_906 => w_12w5_906,
                          w_12w5_907 => w_12w5_907,
                          w_12w5_908 => w_12w5_908,
                          w_12w5_91 => w_12w5_91,
                          w_12w5_92 => w_12w5_92,
                          w_12w5_93 => w_12w5_93,
                          w_12w5_94 => w_12w5_94,
                          w_12w5_95 => w_12w5_95,
                          w_12w5_96 => w_12w5_96,
                          w_12w5_97 => w_12w5_97,
                          w_12w5_98 => w_12w5_98,
                          w_12w5_99 => w_12w5_99,
                          w_12w6_90 => w_12w6_90,
                          w_12w6_900 => w_12w6_900,
                          w_12w6_901 => w_12w6_901,
                          w_12w6_903 => w_12w6_903,
                          w_12w6_904 => w_12w6_904,
                          w_12w6_905 => w_12w6_905,
                          w_12w6_906 => w_12w6_906,
                          w_12w6_91 => w_12w6_91,
                          w_12w6_92 => w_12w6_92,
                          w_12w6_93 => w_12w6_93,
                          w_12w6_94 => w_12w6_94,
                          w_12w6_95 => w_12w6_95,
                          w_12w6_96 => w_12w6_96,
                          w_12w6_97 => w_12w6_97,
                          w_12w6_98 => w_12w6_98,
                          w_12w6_99 => w_12w6_99,
                          w_12w7_90 => w_12w7_90,
                          w_12w7_900 => w_12w7_900,
                          w_12w7_901 => w_12w7_901,
                          w_12w7_902 => w_12w7_902,
                          w_12w7_903 => w_12w7_903,
                          w_12w7_904 => w_12w7_904,
                          w_12w7_905 => w_12w7_905,
                          w_12w7_906 => w_12w7_906,
                          w_12w7_907 => w_12w7_907,
                          w_12w7_908 => w_12w7_908,
                          w_12w7_91 => w_12w7_91,
                          w_12w7_92 => w_12w7_92,
                          w_12w7_93 => w_12w7_93,
                          w_12w7_94 => w_12w7_94,
                          w_12w7_95 => w_12w7_95,
                          w_12w7_96 => w_12w7_96,
                          w_12w7_97 => w_12w7_97,
                          w_12w7_98 => w_12w7_98,
                          w_12w7_99 => w_12w7_99,
                          w_12w8_90 => w_12w8_90,
                          w_12w8_900 => w_12w8_900,
                          w_12w8_901 => w_12w8_901,
                          w_12w8_903 => w_12w8_903,
                          w_12w8_904 => w_12w8_904,
                          w_12w8_905 => w_12w8_905,
                          w_12w8_908 => w_12w8_908,
                          w_12w8_91 => w_12w8_91,
                          w_12w8_92 => w_12w8_92,
                          w_12w8_93 => w_12w8_93,
                          w_12w8_94 => w_12w8_94,
                          w_12w8_95 => w_12w8_95,
                          w_12w8_96 => w_12w8_96,
                          w_12w8_97 => w_12w8_97,
                          w_12w8_98 => w_12w8_98,
                          w_12w8_99 => w_12w8_99);
   --  This process does the real job.
   -- purpose: Read the test script and pass it to the UUT
   -- type   : combinational
   -- inputs : 
   -- outputs: 
   test: process
     constant idle : coaxsigs := ('0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0');
     variable testdata : testvec; -- One line worth of test data
     variable l : line;
     file vector_file : text is in "./cdc_tb.txt";  -- test vector file
     variable g : boolean;
     variable b : character;
     variable i : integer;
     variable d : integer;
     variable ic, oc : coaxsigs;
     variable c1, c2, c3, c4 : integer := 0;
     variable count4 : integer := 4;
     variable count40 : integer := 4;
     variable x, y : integer := 0;
     variable unblank : integer := 0;
     constant space : string := " ";
     constant zero : string := "0";
     constant one : string := "1";
     variable llen : integer;
   begin  -- process test
     dtmain;
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
         oc(i) := testdata(i);
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
         count4 := count4 - 1;
         if count4 < 1 then
           count4 := 4;
           w_12w2_906 <= '1';
           x := TO_INTEGER (w_12w6_905);
           y := TO_INTEGER (w_12w6_903);
           if w_12w6_98 = '1' then
             unblank := 1;
           else
             unblank := 0;
           end if;
           write (l, x);
           write (l, space);
           write (l, y);
           write (l, space);
           write (l, unblank);
           writeline (output, l);
         else
           w_12w2_906 <= '0';           
         end if;
         count40 := count40 - 1;
         if count40 < 1 then
           count40 := 40;
           w_12w2_907 <= '1';
         else
           w_12w2_907 <= '0';           
         end if;
         w_12w1_90 <= oc(1);
         w_12w1_91 <= oc(2);
         w_12w1_92 <= oc(3);
         w_12w1_93 <= oc(4);
         w_12w1_94 <= oc(5);
         w_12w1_95 <= oc(6);
         w_12w1_96 <= oc(7);
         w_12w1_97 <= oc(8);
         w_12w1_98 <= oc(9);
         w_12w1_99 <= oc(10);
         w_12w1_900 <= oc(11);
         w_12w1_901 <= oc(12);
         w_12w1_902 <= oc(13);
         w_12w1_903 <= oc(14);
         w_12w1_904 <= oc(15);
         w_12w1_905 <= oc(16);
         w_12w1_906 <= oc(17);
         w_12w1_907 <= oc(18);
         -- w_12w1_908 <= oc(19);
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
         oc := idle;
       end loop;  -- i
     end loop;
     assert false report "end of test";
     --  Wait forever; this will finish the simulation.
     wait;
   end process test;
end;

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

--  A testbench has no ports.

entity cdc_tb is
end cdc_tb;

architecture behav of cdc_tb is
   --  Declaration of the component that will be instantiated.
   component latch
     port (
       d, clk : in  std_logic;                   -- data (set), clock
       q, qb  : out std_logic);                  -- q and q.bar
   end component;
   --  Specifies which entity is bound with the component.
   for cdc6600_0: latch use entity work.latch;
   signal d, clk, q, qb : std_logic;
begin
   --  Component instantiation.
   cdc6600_0: latch port map (d => d, clk => clk, q => q, qb => qb);

   --  This process does the real job.
   process
      type pattern_type is record
         --  The inputs of the cdc6600.
         i0, i1 : std_logic;
         --  The expected outputs of the cdc6600.
         --o1, o2 : std_logic;
      end record;
      --  The patterns to apply.
      type pattern_array is array (natural range <>) of pattern_type;
      constant patterns : pattern_array :=
        (('0', '0'),
         ('0', '0'),
         ('0', '1'),
         ('0', '0'),
         ('0', '0'),
         ('0', '0'),
         ('1', '0'),
         ('1', '0'),
         ('1', '1'),
         ('1', '0'),
         ('1', '0'),
         ('1', '0'),
         ('0', '0'),
         ('0', '0'),
         ('0', '1'),
         ('0', '0'),
         ('0', '0'),
         ('0', '0'),
         ('1', '0'),
         ('1', '0'),
         ('1', '1'),
         ('1', '0'),
         ('1', '0'),
         ('1', '0'),
         ('0', '0'),
         ('0', '0'),
         ('0', '1'),
         ('0', '0'),
         ('0', '0'),
         ('0', '0'),
         ('1', '0'),
         ('1', '0'),
         ('1', '1'),
         ('1', '0'),
         ('1', '0'),
         ('1', '0'),
         ('1', '0'));
   begin
      --  Check each pattern.
      for i in patterns'range loop
        --  Set the inputs.
        d <= patterns(i).i0;
        clk <= patterns(i).i1;
        --  Wait for the results.
        wait for 10 ns;
      end loop;
      assert false report "end of test" severity note;
      --  Wait forever; this will finish the simulation.
      wait;
   end process;
end behav;

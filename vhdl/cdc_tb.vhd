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

package teststuff is

  procedure send (
    func : in  boolean;                 -- true if sending function
    dat  : in  integer;                 -- data to send
    oc   : inout coaxsigs);             -- output cable

end teststuff;

package body teststuff is

  -- purpose: construct output cable signals for function or data
  procedure send (
    func : in    boolean;
    dat  : in    integer;
    oc   : inout coaxsigs) is
    variable i, b, d : integer;
  begin  -- send

    for i in 1 to 19 loop
      oc(i) := '1';
    end loop;  -- i

    b := 2048;
    d := dat;
    for i in 11 downto 0 loop
      if d >= b then
        oc(i + 1) := '0';
        d := d - b;
      end if;
      b := b / 2;
    end loop;  -- i

    if func then
      oc(17) := '0';
    else
      oc(15) := '0';
    end if;
  end send;

end teststuff;

--  A testbench has no ports.

library IEEE;
use IEEE.std_logic_1164.all;
use std.textio.all;
use work.sigs.all;
use work.teststuff.all;

entity cdc_tb is
end cdc_tb;

architecture behav of cdc_tb is
   --  Declaration of the component that will be instantiated.
  component cdc6600 
    port (
      clk : in std_logic;
      coax1_1_extern_1 : inout coaxsigs);
  end component;
  component synchro
    generic (
      chnum : integer);                   -- connected channel number

    port (
      clk10 : in std_logic;             -- 10 MHz clock
      ic : inout coaxsigs;                -- input cable
      oc : inout coaxsigs);               -- output cable
  end component;
   --  Specifies which entity is bound with the component.
  signal coax1 : coaxsigs;              -- First coax
  signal clk : std_logic := '0';        -- clock source

  constant idle : coaxsigs := ('1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1');
  signal ics, ocs : coaxsigs := idle;
  signal clk10 : std_logic := '1';
  type testvec is array (1 to 80) of std_logic;
begin
   --  Component instantiation.
   uut: cdc6600 port map (coax1_1_extern_1 => coax1,
                          clk => clk);
   console : synchro generic map (
     chnum => 8#10#)
   port map (
     clk10 => clk10,
     ic => ics,
     oc => ocs);
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
     procedure sync (
       oc : inout coaxsigs) is
     begin  -- sync
       clk10 <= '0';
       wait for 25 ns;
       clk10 <= '1';
       ocs <= oc;
       wait for 25 ns;
       ocs <= idle;
       wait for 50 ns;
     end sync;
   begin  -- process test
     dtmain;
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
       sync (oc);
       for i in 1 to d loop
         clk <= '1';
         wait for 12.5 ns;
         clk <= '0';
         wait for 12.5 ns;
       end loop;  -- i
     end loop;
     loop
       send (true, 8#7001#, oc);
       sync (oc);
       for i in 1 to 19 loop
         oc(i) := '1';
       end loop;  -- i
       oc(13) := '0';                   -- active
       sync (oc);
       send (false, 8#7400#, oc);
       sync (oc);
       send (false, 8#6060#, oc);
       sync (oc);
       send (false, 8#1005#, oc);
       sync (oc);
       send (false, 8#1414#, oc);
       sync (oc);
       send (false, 8#1755#, oc);
       sync (oc);
       send (false, 8#2610#, oc);
       sync (oc);
       send (false, 8#0414#, oc);
       sync (oc);
       send (false, 8#5555#, oc);
       sync (oc);
       send (false, 8#3333# + (c4 * 64) + c3, oc);
       sync (oc);
       send (false, 8#3333# + (c2 * 64) + c1, oc);
       sync (oc);
       if c1 < 7 then
         c1 := c1 + 1;
       else
         c1 := 0;
         if c2 < 7 then
           c2 := c2 + 1;
         else
           c2 := 0;
           if c3 < 7 then
             c3 := c3 + 1;
           else
             c3 := 0;
             if c4 < 7 then
               c4 := c4 + 1;
             else
               c4 := 0;
             end if;
           end if;
         end if;
       end if;
       for i in 1 to 19 loop
         oc(i) := '1';
       end loop;  -- i
       oc(14) := '0';                   -- inactive
       sync (oc);
       send (true, 8#7020#, oc);
       sync (oc);
       for i in 1 to 19 loop
         oc(i) := '1';
       end loop;  -- i
       oc(13) := '0';                   -- active
       sync (oc);
       for i in 1 to 19 loop
         oc(i) := '1';
       end loop;  -- i
       sync (oc);
       for i in 1 to 19 loop
         oc(i) := '1';
       end loop;  -- i
       oc(14) := '0';                   -- inactive
       sync (oc);
     end loop;
      assert false report "end of test";
      --  Wait forever; this will finish the simulation.
      wait;
   end process test;
end;

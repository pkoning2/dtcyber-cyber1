-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

use work.sigs.all;

entity cmbank is
  generic (
    banknum : integer);                 -- bank number
  port (
    go                     : in  coaxsig;
    addr                   : in  ppword;     -- memory address (12 bits)
    baddr                  : in  bankaddr;   -- bank address (5 bits)
    clk1, clk2, clk3, clk4 : in  std_logic;  -- clocks
    reset                  : in  std_logic;  -- reset
    write                  : in  std_logic;  -- write request
    wdata                  : in  cpword;     -- write data bus
    rdata                  : out cpword;     -- read data bus
    accept                 : out coaxsig);   -- accept signal

end cmbank;

architecture cmbank of cmbank is
  component memarray60
--    generic (
--      idata : ippmem);
    port (
      addr   : in  ppword;                -- Memory address
      rdata  : out cpword;                -- read data
      wdata  : in  cpword;                -- write data
      reset  : in  std_logic;             -- power-up reset
      strobe : in  std_logic;             -- read/write strobe
      write  : in  std_logic);            -- write operation
  end component;
  signal maddr : ppword;                -- 12 bit address
  signal trdata, twdata : cpword;       -- copies of read and write data
  signal twrite : std_logic := '0';     -- write enable
  constant bnum : bankaddr := TO_UNSIGNED (banknum, 5);
begin  -- cmbank  
  mem : memarray60 port map (
    addr   => maddr,
    rdata  => trdata,
    wdata  => twdata,
    strobe => clk2,
    write  => twrite);
  -- purpose: storage sequence state machine
  -- type   : sequential
  -- inputs : clk1, reset, baddr, go
  -- outputs: accept, twrite
  ssc: process (clk1, reset)
    variable seq : integer := 0;        -- sequence counter
    variable acc, tw1, tw2 : std_logic := '0';
  begin  -- process ssc
    if reset = '0' then                 -- asynchronous reset (active low)
      seq := 0;
      tw1 := '0';
      tw2 := '0';
      acc := '0';
    elsif clk1'event and clk1 = '1' then  -- rising clock edge
      if (seq = 0) and (go = '1') and (baddr = bnum) then
        seq := 1;
      elsif seq = 9 then
        seq := 0;
      else
        seq := seq + 1;
      end if;
      if seq = 2 then
        acc := '1';
      else
        acc := '0';
      end if;
      if seq = 4 then
        tw1 := write;
      elsif seq = 6 then
        tw1 := '0';
      end if;
      if seq = 5 then
        tw2 := tw1;
      else
        tw2 := '0';
      end if;
      accept <= acc and clk1;
      twrite <= tw2;
    end if;
  end process ssc;
  twdata <= wdata when (twrite = '1') and (clk1 = '1') else unaffected;
end cmbank;

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

use work.sigs.all;

entity cmem is
  
  port (
    go                             : in  coaxsig;  -- go from stunt box
    addr                           : in  coaxsigs;  -- memory address from stunt box
    wdata1, wdata2, wdata3, wdata4 : in  coaxsigs;  -- write data trunk cables
    write                          : in  coaxsig;  -- write request
    periph, ecs                    : in  coaxsig;  -- read data routing signals
    rdctrl                         : out coaxsigs;  -- read data to control
    rdecs1, rdecs2, rdecs3, rdecs4 : out coaxsigs;  -- read data to ecs
    rdregl1, rdregl2, rdregl3      : out coaxsigs;  -- read data to lower regs
    rdregu1, rdregu2               : out coaxsigs;  -- read data to upper regs
    rdpp1, rdpp2, rdpp3, rdpp4     : out coaxsigs;  -- read data to pp pyramid
    accept                         : out coaxsig;  -- accepts to stunt box
    clk1, clk2, clk3, clk4         : in  std_logic);  -- clocks

end cmem;

architecture beh of cmem is
  signal taddr : ppword;
  signal bank : bankaddr;
begin  -- beh
  -- Unswizzle the address cable
  taddr(0) <= addr(16);
  taddr(1) <= addr(17);
  taddr(2) <= addr(18);
  taddr(3) <= addr(0);
  taddr(4) <= addr(1);
  taddr(5) <= addr(2);
  taddr(6) <= addr(3);
  taddr(7) <= addr(4);
  taddr(8) <= addr(5);
  taddr(9) <= addr(6);
  taddr(10) <= addr(7);
  taddr(11) <= addr(8);
  bank(0) <= addr(11);
  bank(1) <= addr(12);
  bank(2) <= addr(13);
  bank(3) <= addr(14);
  bank(4) <= addr(15);
  -- Unswizzle the write data cables
  -- Swizzle the read data for the output trunks

end beh;

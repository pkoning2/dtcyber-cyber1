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
-- Behavioral model of CM.  This is the entire CM, all
-- banks of it.  We don't bother trying to model this at
-- the module level, that just adds a lot of detail for
-- no great benefit.
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.numeric_bit.all;
use work.sigs.all;


-- cmarray is a 60 bit memory array, with a single read and write port.
-- it models the 5 12-bit memory arrays (though inside it's actually
-- 8 8-bit arrays, since that's what the FPGA provides)

entity cmarray is
  port (
    addr   : in  ppword;                -- memory address
    rdata  : out cpword;                -- read data
    wdata  : in  cpword;                -- write data
    reset  : in  logicsig;              -- power-up reset
    clk    : in  logicsig;              -- memory clock
    ena    : in  logicsig;              -- enable
    write  : in  logicsig);             -- write request
end cmarray;

architecture beh of cmarray is
  component memarray is
    generic (
      abits : integer := 12;              -- number of address bits
      dbits : integer := 8);              -- number of data bits
    port (
      addr_a  : in  UNSIGNED(abits - 1 downto 0);  -- port A address
      rdata_a : out UNSIGNED(dbits - 1 downto 0);  -- port A data out
      wdata_a : in  UNSIGNED(dbits - 1 downto 0);  -- port A data in
      clk_a   : in  logicsig;                      -- port A clock
      write_a : in  logicsig;                      -- port A write enable
      ena_a   : in  logicsig;                      -- port A enable
      addr_b  : in  UNSIGNED(abits - 1 downto 0) := (others => '0');  -- port B address
      rdata_b : out UNSIGNED(dbits - 1 downto 0) := (others => '0');  -- port B data out
      wdata_b : in  UNSIGNED(dbits - 1 downto 0) := (others => '0');  -- port B data in
      clk_b   : in  logicsig := '0';               -- port B clock
      write_b : in  logicsig := '0';               -- port B write enable
      ena_b   : in  logicsig := '0';               -- port B enable
      reset   : in  logicsig);                     -- power-up reset
  end component;
  signal trdata, twdata : UNSIGNED(63 downto 0);
begin  -- beh
  twdata <= "0000" & wdata;
  arrays: for bank in 0 to 7 generate
    membank : memarray port map (
      addr_a  => addr,
      rdata_a => trdata(63 - (bank * 8) downto 56 - (bank * 8)),
      wdata_a => twdata(63 - (bank * 8) downto 56 - (bank * 8)),
      clk_a   => clk,
      ena_a   => ena,
      write_a => write,
      reset   => reset);
  end generate arrays;
  rdata <= trdata(59 downto 0);
end beh;


library IEEE;
use IEEE.numeric_bit.all;
use work.sigs.all;

entity cmbank is
  generic (
    banknum : integer);                 -- bank number
  port (
    go                     : in  coaxsig;
    addr                   : in  ppword;     -- memory address (12 bits)
    baddr                  : in  bankaddr;   -- bank address (5 bits)
    clk1, clk2, clk3, clk4 : in  logicsig;   -- clocks
    reset                  : in  logicsig;   -- reset
    write                  : in  logicsig;   -- write request
    wdata                  : in  cpword;     -- write data bus
    rdata                  : out cpword;     -- read data bus
    accept                 : out coaxsig);   -- accept signal

end cmbank;

architecture cmbank of cmbank is
  component cmarray is
    port (
      addr   : in  ppword;                -- memory address
      rdata  : out cpword;                -- read data
      wdata  : in  cpword;                -- write data
      reset  : in  logicsig;              -- power-up reset
      clk    : in  logicsig;              -- memory clock
      ena    : in  logicsig;              -- enable
      write  : in  logicsig);             -- write request
  end component;

  signal maddr : ppword;                -- 12 bit address
  signal trdata, twdata : cpword;       -- copy of read and write data
  signal tena : logicsig := '0';        -- write control to memory
  signal twrite : logicsig := '0';      -- write control to memory
  constant bnum : bankaddr := TO_UNSIGNED (banknum, 5);
  signal seq, next_seq : natural range 0 to 9 := 0; -- sequencer state
  signal do_write, writereq : boolean := false;     -- true if write requested
begin  -- cmbank  
  mem : cmarray port map (
    addr   => maddr,
    rdata  => trdata,
    wdata  => twdata,
    clk    => clk4,
    ena    => tena,
    write  => twrite);

  -- purpose: state machine for next state and other outputs from storage sequencer
  -- type   : combinational
  -- inputs : seq, go, write
  -- outputs: next_seq, do_write
  ssc_next: process (seq, write)
  begin  -- process ssc_next
    -- start with some default outputs
    do_write <= false;
    case seq is
      when 0 =>
        if go = '1' and baddr = bnum then
          next_seq <= 1;
        else
          next_seq <= seq;              -- no request for this bank, stay in t0
        end if;
      when 2 =>
        if write = '1' then
          do_write <= true;
        end if;
      when 9 =>
        next_seq <= 0;
      when others =>
        next_seq <= seq + 1;
    end case;
  end process ssc_next;
  -- purpose: storage sequence machine
  -- type   : combinational
  -- inputs : clk1, next_seq, do_write
  -- outputs: seq, writereq
  ssc: process (clk1)
  begin  -- process ssc
    if clk1'event and clk1 = '1' then
      case next_seq is
        when 1 =>
          maddr <= addr;
        when 3 =>
          writereq <= do_write;
        when 5 =>
          twdata <= wdata;
        when others => null;
      end case;
      seq <= next_seq;
    end if;
  end process ssc;
  accept <= '1' when seq = 1 and clk1 = '1' else '0';
  tena <= '1' when seq = 4 or (seq = 6 and writereq);
  twrite <= '1' when seq = 6 and writereq;
  rdata <= trdata when seq = 5 and clk1 = '1' else (others => '0');
end cmbank;

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

use work.sigs.all;

entity cmem is
  
  port (
    go                             : in  coaxsig;  -- go from stunt box
    addr                           : in  coaxsigs; -- memory address from stunt box
    wdata1, wdata2, wdata3, wdata4 : in  coaxsigs;  -- write data trunk cables
    write                          : in  coaxsig;  -- write request
    periph, ecs                    : in  coaxsig;  -- read data routing signals
    rdctrl                         : out coaxsigs;  -- read data to control
    rdecs1, rdecs2, rdecs3, rdecs4 : out coaxsigs;  -- read data to ecs
    rdregl1, rdregl2, rdregl3      : out coaxsigs;  -- read data to lower regs
    rdregu1, rdregu2               : out coaxsigs;  -- read data to upper regs
    rdpp1, rdpp2, rdpp3, rdpp4     : out coaxsigs;  -- read data to pp pyramid
    accept                         : out coaxsig;  -- accepts to stunt box
    clk1, clk2, clk3, clk4         : in  logicsig);  -- clocks

end cmem;

architecture beh of cmem is
  component cmbank is
    generic (
      banknum : integer);                 -- bank number
    port (
      go                     : in  coaxsig;
      addr                   : in  ppword;     -- memory address (12 bits)
      baddr                  : in  bankaddr;   -- bank address (5 bits)
      clk1, clk2, clk3, clk4 : in  logicsig;  -- clocks
      reset                  : in  logicsig;  -- reset
      write                  : in  logicsig;  -- write request
      wdata                  : in  cpword;     -- write data bus
      rdata                  : out cpword;     -- read data bus
      accept                 : out coaxsig);   -- accept signal
  end component;
  type rvec_t is array (0 to 31) of cpword;
  type acc_t is array (0 to 31) of coaxsig;
  signal laddr : coaxsigs;
  signal taddr : ppword;
  signal bank : bankaddr;
  signal twdata : cpword;
  signal trdata : rvec_t;               -- read contributions from banks
  signal rdata : cpword;                -- merged read data to trunks
  signal taccept : acc_t;               -- accept contributions from banks
begin  -- beh
  -- Latch and unswizzle the address cable (from stunt box, chassis 5 Q34-Q39)
  alatch: process (addr, clk4)
  begin  -- process alatch
    if clk4 = '1' then
      laddr <= (others => '0');
    end if;
    for i in addr'range loop
      if addr(i) = '1' then
        laddr(i) <= '1';
      end if;
    end loop;
  end process alatch;
  taddr <= (laddr(8), laddr(7), laddr(6), laddr(5), laddr(4), laddr(3),
            laddr(2), laddr(1), laddr(0), laddr(18), laddr(17), laddr(16));
  bank <= (laddr(15), laddr(14), laddr(13), laddr(12), laddr(11));

  -- latch and unswizzle the write data cables (from store distributor,
  -- chassis 2 B12-B21)
  wlatch: process (wdata1, wdata2, wdata3, wdata4, clk4)
  begin  -- process alatch
    if clk4 = '1' then
      twdata <= (others => '0');
    end if;
    for i in 0 to 14 loop
      if wdata1(i) = '1' then
        twdata(i) <= '1';
      end if;
      if wdata2(i) = '1' then
        twdata(i + 15) <= '1';
      end if;
      if wdata3(i) = '1' then
        twdata(i + 30) <= '1';
      end if;
      if wdata4(i) = '1' then
        twdata(i + 45) <= '1';
      end if;
    end loop;
  end process wlatch;

  -- 32 memory banks, 4k by 60 each
  mbank: for b in 0 to 31 generate
    cm : cmbank
      generic map (
        banknum => b)
      port map (
        go     => go,
        addr   => taddr,
        baddr  => bank,
        clk1   => clk1,
        clk2   => clk2,
        clk3   => clk3,
        clk4   => clk4,
--        reset  => reset,
        write  => write,
        wdata  => twdata,
        rdata  => trdata(b),
        accept => taccept(b));
  end generate mbank;
  
  -- merge bank contributions
  trunks: process (trdata, taccept)
    variable ttrdata : cpword;
    variable ttaccept : coaxsig;
  begin  -- process trunks
    ttrdata := (others => '0');
    ttaccept := '0';
    for i in trdata'range loop
      for j in cpword'range loop
        ttrdata(j) := ttrdata(j) or trdata(i)(j);
      end loop;  -- j
      ttaccept := ttaccept or taccept(i);
    end loop;  -- i
    
    accept <= ttaccept;
    rdata <= ttrdata;
  end process trunks;

  -- Swizzle the read data for the output trunks

end beh;

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
  generic (
    bnum  : integer := 0);             -- bank number for meminit
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
      bnum  : integer := 0;             -- bank number for meminit
      abits : integer := 12);             -- number of address bits
    port (
      addr_a  : in  logicbus(abits - 1 downto 0);  -- port A address
      rdata_a : out logicbus(7 downto 0);  -- port A data out
      wdata_a : in  logicbus(7 downto 0);  -- port A data in
      clk_a   : in  logicsig;                      -- port A clock
      write_a : in  logicsig;                      -- port A write enable
      ena_a   : in  logicsig;                      -- port A enable
      addr_b  : in  logicbus(abits - 1 downto 0) := (others => '0');  -- port B address
      rdata_b : out logicbus(7 downto 0) := (others => '0');  -- port B data out
      wdata_b : in  logicbus(7 downto 0) := (others => '0');  -- port B data in
      clk_b   : in  logicsig := '0';               -- port B clock
      write_b : in  logicsig := '0';               -- port B write enable
      ena_b   : in  logicsig := '0';               -- port B enable
      reset   : in  logicsig);                     -- power-up reset
  end component;
  signal trdata, twdata : logicbus(63 downto 0);
begin  -- beh
  twdata <= "0000" & wdata;
  arrays: for bank in 0 to 7 generate
    membank : memarray generic map (
      bnum => bnum * 8 + bank + 100)
    port map (
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
    generic (
      bnum  : integer := 0);             -- bank number for meminit
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
  constant bnum : UNSIGNED := TO_UNSIGNED (banknum, 5);
  signal seq, next_seq : natural range 0 to 9 := 0; -- sequencer state
  signal do_write, writereq : boolean := false;     -- true if write requested
begin  -- cmbank  
  mem : cmarray generic map (
    bnum => banknum)
   port map (
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
        if go = '1' and UNSIGNED (baddr) = bnum then
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
  tena <= '1' when seq = 4 or (seq = 6 and writereq) else '0';
  twrite <= '1' when seq = 6 and writereq else '0';
  rdata <= trdata when seq = 5 and clk1 = '1' else (others => '0');
end cmbank;

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

use work.sigs.all;

entity cpmem is
  
  port (
    p1                 : in coaxsigs;   -- go, write, etc from stunt box
    p2                 : in coaxsigs;   -- address from stunt box
    p3, p4, p5, p6     : in coaxsigs;   -- write data trunk
    p7, p8, p9, p10    : out coaxsigs;  -- read trunk to control
    p11, p12, p13, p14 : out coaxsigs;  -- read data trunk to ecs
    p15, p16, p17      : out coaxsigs;  -- read data trunk to lower regs
    p18, p19           : out coaxsigs;  -- read data trunk to upper regs
    p20, p21, p22, p23 : out coaxsigs;  -- read data trunk to ppu
    p24                : out coaxsigs;  -- accept to stunt box
    clk1, clk2, clk3, clk4         : in  logicsig);  -- clocks

end cpmem;

architecture beh of cpmem is
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
  component ireg 
    port (
      clr : in bit;                       -- clear pulse
      ibus : in coaxbus;                  -- input bus
      obus : out coaxbus);                -- output bus
  end component;
  type rvec_t is array (0 to 31) of cpword;
  type acc_t is array (0 to 31) of coaxsig;
  subtype coaxword is coaxbus (59 downto 0);  -- cpword, coax signal type
  alias go : coaxsig is p1(11);         -- go from stunt box
  alias write : coaxsig is p1(12);      -- write from stunt box
  alias periph : coaxsig is p1(14);     -- peripheral read from stunt box
  alias ecs : coaxsig is p1(15);        -- ecs read from stunt box
  alias addr : coaxsigs is p2;          -- address from stunt box
  alias wdata1 : coaxsigs is p3;        -- write data trunk
  alias wdata2 : coaxsigs is p4;
  alias wdata3 : coaxsigs is p5;
  alias wdata4 : coaxsigs is p6;
  alias rdctrl1 : coaxsigs is p7;       -- read data to control
  alias rdctrl2 : coaxsigs is p8;
  alias rdctrl3 : coaxsigs is p9;
  alias rdctrl4 : coaxsigs is p10;
  alias rdecs1 : coaxsigs is p11;       -- read data to ecs
  alias rdecs2 : coaxsigs is p12;
  alias rdecs3 : coaxsigs is p13;
  alias rdecs4 : coaxsigs is p14;
  alias rdregl1 : coaxsigs is p15;      -- read data to lower registers
  alias rdregl2 : coaxsigs is p16;
  alias rdregl3 : coaxsigs is p17;
  alias rdregu1 : coaxsigs is p18;      -- read data to upper registers
  alias rdregu2 : coaxsigs is p19;
  alias rdpp1 : coaxsigs is p20;        -- read data to pp read pyramid
  alias rdpp2 : coaxsigs is p21;
  alias rdpp3 : coaxsigs is p22;
  alias rdpp4 : coaxsigs is p23;
  alias accept : coaxsig is p24(9);     -- accept to stunt box
  signal laddr : coaxsigs;
  signal taddr : ppword;
  signal bank : bankaddr;
  signal iwdata, lwdata : coaxword;
  signal twdata : cpword;
  signal trdata : rvec_t;               -- read contributions from banks
  signal rdata : coaxword;                -- merged read data to trunks
  signal taccept : acc_t;               -- accept contributions from banks
  signal maccept : coaxsig;             -- merged accept
  signal mrdata : cpword;               -- merged read data
begin  -- beh
  -- Latch and unswizzle the address cable (from stunt box, chassis 5 Q34-Q39)
  alatch : ireg port map (
    ibus => addr,
    clr  => clk4,
    obus => laddr);
  -- chassis 4I20
  -- write is w19-902
  -- go is w19-901
  -- read periph is w19-904
  taddr <= (laddr(8), laddr(7), laddr(6), laddr(5), laddr(4), laddr(3),
            laddr(2), laddr(1), laddr(0), laddr(18), laddr(17), laddr(16));
  bank  <= (laddr(15), laddr(14), laddr(13), laddr(12), laddr(11));

  -- latch the write data cables (from store distributor,
  -- chassis 2 B12-B21)
  iwdata <= wdata4 (14 downto 0) & wdata3 (14 downto 0) &
            wdata2 (14 downto 0) & wdata1 (14 downto 0);
  wlatch : ireg port map (
    ibus => iwdata,
    clr  => clk4,
    obus => lwdata);
  twdata <= cpword (lwdata);
  
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
    rdata <= coaxword (ttrdata);
  end process trunks;

  -- Swizzle the read data for the output trunks
  -- chassis 1 cm to pyramid:
  -- 0..14 W04-90..904, 15..29 W05-90.904
  -- 30..37 W06-900..907, 38..44 W06-90..96, 45..52 W07-900..907,
  -- 53..59 W07-90..96
  -- ***TODO: need to AND with pp read data output enable
  rdpp1 (14 downto 0) <= rdata (14 downto 0);
  rdpp2 (14 downto 0) <= rdata (29 downto 15);
  rdpp3 (17 downto 10) <= rdata (37 downto 30);
  rdpp3 (6 downto 0) <= rdata (44 downto 38);
  rdpp4 (17 downto 10) <= rdata (52 downto 45);
  rdpp4 (6 downto 0) <= rdata (59 downto 53);
  
  -- chassis 5 input register (A-E 41,42):
  -- 0..3 W02-904..907, 4 W02-900 5..14 W02-90..99
  -- 15..18 W01-904..907, 19 W01-900, 20..29 W01-90..99
  -- 30..37 W04-900..907, 38..44 W04-90..90, 
  -- 45..52 W03-900..907, 53..59 W03-90..96, 
  rdctrl1 (17 downto 14) <= rdata (3 downto 0);
  rdctrl1 (10) <= rdata (4);
  rdctrl1 (9 downto 0) <= rdata (14 downto 5);
  rdctrl2 (17 downto 14) <= rdata (18 downto 15);
  rdctrl2 (10) <= rdata (19);
  rdctrl2 (9 downto 0) <= rdata (29 downto 20);
  rdctrl3 (17 downto 14) <= rdata (33 downto 30);
  rdctrl3 (10) <= rdata (34);
  rdctrl3 (9 downto 0) <= rdata (44 downto 35);
  rdctrl4 (17 downto 14) <= rdata (48 downto 45);
  rdctrl4 (10) <= rdata (49);
  rdctrl4 (9 downto 0) <= rdata (59 downto 50);

  -- ECS is done the easy way:
  rdecs1 (14 downto 0) <= rdata (14 downto 0);
  rdecs2 (14 downto 0) <= rdata (29 downto 15);
  rdecs3 (14 downto 0) <= rdata (44 downto 30);
  rdecs4 (14 downto 0) <= rdata (59 downto 45);

  -- chassis 7 entry trunk (A-C 37-42): (register bits 0..35)
  -- 0..7 W05-900..907, 8..14 W05-90..96
  -- 15..22 W06-900..907, 23..29 W06-90..96
  -- 30..35 W07-900..905
  rdregl1 (17 downto 10) <= rdata (7 downto 0);
  rdregl1 (6 downto 0) <= rdata (14 downto 8);
  rdregl2 (17 downto 10) <= rdata (22 downto 15);
  rdregl2 (6 downto 0) <= rdata (29 downto 23);
  rdregl3 (15 downto 10) <= rdata (35 downto 30);
  
  -- chassis 8 memory trunk (A01-08, B01-04): (register bits 36..59)
  -- 36..44 W05-900..908, 45..59 W06-90..904
  rdregu1 (18 downto 10) <= rdata (44 downto 36);
  rdregu2 (14 downto 0) <= rdata (59 downto 45);
  
end beh;

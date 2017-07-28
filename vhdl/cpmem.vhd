-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- Behavioral model of CM.  This is the entire CM, all banks of it.
-- We don't bother trying to model this at the module level, that just
-- adds a lot of detail for no great benefit.  It also includes the
-- read and write data distributor aspects.  This is partly because
-- the read distributor wire lists aren't available, but mostly
-- because both distributors add an extravagant amount of logic that
-- does not actually contribute any meaningful functionality.
--
-------------------------------------------------------------------------------

--
-- Summary of interface timing (from/to the rest of the 6600 model).
-- Times are given in nanoseconds relative to a somewhat arbitrary
-- origin (apparently the time when the memory reference request is
-- entered into the stunt box).  Much of this comes from the memory
-- timing chart in CDC manual 60147400 (6600 Training Manual), along
-- with the CPU and memory block diagrams.  Note that the timing is as
-- seen by the cpmem component, which subsumes various fanout and data
-- distributor components that the manual describe separately.  For
-- example "go" as seen here precedes the address, because we get
-- "go" direct from the stunt box in chassis 5 rather than by way of
-- a "go" fanout block in chassis 4.
--
-- These times are from the receiving end point of view.  Note that in
-- the gate level models the coax delay is modeled as a separate delay
-- step at the sending end; here we model it directly by gating the
-- output signals with the appropriate clock phase.
--
-- Inputs to cpmem:
-- Go:          T195
-- Address:     T255
-- Write:       T255
-- Tag decodes: T455
-- Write data:  T600 ?
--
-- Outputs from cpmem:
-- Accept:      T370
-- Read data:   T645
-- Read resume: T645 ?
--
-- Sequence controller sequence numbers: The SSC state machine
-- advances on T70, starting with sequence number 1 on the sysclk(14)
-- rising edge following the address.  So sequence 1 corresponds to
-- T270.

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
  arrays: for bytenum in 0 to 7 generate
    membank : memarray generic map (
      bnum => bnum * 8 + bytenum + 100)
    port map (
      addr_a  => addr,
      rdata_a => trdata(63 - (bytenum * 8) downto 56 - (bytenum * 8)),
      wdata_a => twdata(63 - (bytenum * 8) downto 56 - (bytenum * 8)),
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
    sysclk                 : in  clocks;     -- clocks
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
    reset  => reset,
    addr   => maddr,
    rdata  => trdata,
    wdata  => twdata,
    clk    => sysclk(9),
    ena    => tena,
    write  => twrite);

  -- purpose: state machine for next state and other outputs from storage sequencer
  -- type   : combinational
  -- inputs : seq, go, write
  -- outputs: next_seq, do_write
  ssc_next: process (seq, write, go, baddr)
  begin  -- process ssc_next
    -- start with some default outputs
    do_write <= false;
    case seq is
      when 0 =>
        if go = '1' and UNSIGNED (baddr) = bnum then
          next_seq <= 1;
          if write = '1' then
            do_write <= true;
          end if;
        else
          next_seq <= seq;              -- no request for this bank, stay in t0
        end if;
      when 9 =>
        next_seq <= 0;
      when others =>
        next_seq <= seq + 1;
    end case;
  end process ssc_next;
  -- purpose: storage sequence machine
  -- type   : sequential
  -- inputs : T70, next_seq, do_write
  -- outputs: seq, writereq
  ssc: process (sysclk(14))
    variable send_accept : logicsig := '0';
  begin  -- process ssc
    if rising_edge (sysclk(14)) then
      send_accept := '0';
      case next_seq is
        when 1 =>
          maddr <= addr;
          writereq <= do_write;
        when 2 =>
          send_accept := '1';
        when 4 =>       -- T470
          twdata <= wdata;
        when others =>
          null;
      end case;
      seq <= next_seq;
    end if;
    accept <= send_accept and sysclk(14);
  end process ssc;
      
  twrite <= '1' when seq = 7 and writereq else '0';
  tena <= '1' when seq = 3 or twrite = '1' else '0';
  rdata <= trdata when seq = 4 and sysclk(9) = '1'      -- T645
           else (others => '0');
end cmbank;

library IEEE;
use IEEE.numeric_bit.all;
use IEEE.numeric_std.all;

use work.sigs.all;

entity cpmem is
  
  port (
    p1                 : in coaxsigs;   -- go, write, etc from stunt box
    p2                 : in coaxsigs;   -- address from stunt box
    p3, p4, p5, p6     : in coaxsigs;   -- write data trunk to ppu
    p7, p8, p9, p10    : out coaxsigs;  -- read trunk to control
    p11, p12, p13, p14 : out coaxsigs;  -- read data trunk to ecs
    p15, p16, p17      : out coaxsigs;  -- read data trunk to lower regs
    p18, p19           : out coaxsigs;  -- read data trunk to upper regs
    p20, p21, p22, p23 : out coaxsigs;  -- read data trunk to ppu
    p24                : out coaxsigs;  -- accept to stunt box
    p25                : out coaxsigs;  -- read resume to PP
    p26                : out coaxsigs;  -- write resume to PP
    p101, p102         : in coaxsigs;   -- write trunk from control
    p103, p104, p105, p106 : in coaxsigs; -- write data trunk from ecs
    p107, p108, p109   : in coaxsigs;   -- write data trunk from lower regs
    p110, p111         : in coaxsigs;   -- write data trunk from upper regs
    reset  : in  logicsig;              -- power-up reset
    sysclk             : in  clocks);   -- clocks

end cpmem;

architecture beh of cpmem is
  component cmbank is
    generic (
      banknum : integer);                 -- bank number
    port (
      go                     : in  coaxsig;
      addr                   : in  ppword;     -- memory address (12 bits)
      baddr                  : in  bankaddr;   -- bank address (5 bits)
      sysclk : in  clocks;  -- clocks
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
  alias rdppu : coaxsig is p1(14);      -- peripheral read from stunt box
  alias ecs : coaxsig is p1(15);        -- ecs read from stunt box
  alias addr : coaxsigs is p2;          -- address from stunt box
  alias wrppu1 : coaxsigs is p3;        -- write data from ppu
  alias wrppu2 : coaxsigs is p4;
  alias wrppu3 : coaxsigs is p5;
  alias wrppu4 : coaxsigs is p6;
  alias wrecs1 : coaxsigs is p103;      -- write data from ecs
  alias wrecs2 : coaxsigs is p104;
  alias wrecs3 : coaxsigs is p105;
  alias wrecs4 : coaxsigs is p106;
  alias wrregl1 : coaxsigs is p107;     -- write data from lower regs
  alias wrregl2 : coaxsigs is p108;
  alias wrregl3 : coaxsigs is p109;
  alias wrregu1 : coaxsigs is p110;     -- write data from upper regs
  alias wrregu2 : coaxsigs is p111;
  alias wrctrl1 : coaxsigs is p101;     -- write data from control
  alias wrctrl2 : coaxsigs is p102;
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
  alias c5full : coaxsig is p25(15);    -- set c5 full to PP
  alias rresume : coaxsig is p25(16);   -- read resume to PP
  alias wresume : coaxsig is p26(15);   -- write resume to PP
  alias storectl : coaxsig is p102(12); -- store control from stunt box
  alias storereg : coaxsig is p102(13); -- store register from stunt box
  alias storeppu : coaxsig is p102(14); -- store ppu from stunt box
  signal lctrl : coaxsigs;              -- Latched control wires
  signal laddr : coaxsigs;              -- Latched address cable
  signal lgo, lwrite, lrdppu : coaxsig; -- latched go, write, readpp
  signal lwctl, lwreg, lwppu : coaxsig; -- latched store 
  signal rdppud1, rdppud2 : logicsig;   -- periph read delayed n clks
  signal iwresume : logicsig;           -- internal copy of write resume
  signal taddr : ppword;
  signal bank : bankaddr;
  signal iwppu, iwctrl, iwreg, iwecs : coaxword;
  signal lwpdata : coaxword;            -- latched PP data
  signal lwdata : coaxword;
  signal twdata : cpword;
  signal trdata : rvec_t;               -- read contributions from banks
  signal rdata : coaxword;              -- merged read data to trunks
  signal prdata : coaxword;             -- read data for PP
  signal taccept : acc_t;               -- accept contributions from banks
begin  -- beh
  -- Latch the control signals
  golatch : process (sysclk (2))
  begin  -- process
    if rising_edge (sysclk (2)) then
      lgo <= go;
    end if;
  end process golatch;
  ctlatch : process (sysclk (13))
  begin  -- process
    if rising_edge (sysclk (13)) then
      lwrite <= write;
      lrdppu <= rdppu;
      lwctl <= storectl;
      lwreg <= storereg;
      lwppu <= storeppu;
    end if;
  end process ctlatch;
  
  -- Latch and unswizzle the address cable (from stunt box, chassis 5 Q34-Q39)
  alatch : process (sysclk (13))
  begin  -- process
    if rising_edge (sysclk (13)) then
      laddr <= addr;
    end if;
  end process alatch;
  taddr <= (laddr(8), laddr(7), laddr(6), laddr(5), laddr(4), laddr(3),
            laddr(2), laddr(1), laddr(0), laddr(18), laddr(17), laddr(16));
  bank  <= (laddr(15), laddr(14), laddr(13), laddr(12), laddr(11));

  -- Delay "rdppu" by 2 cycles for use with the read data reply to the PPU.
  ppdelay : process (sysclk(6))
  begin  -- process
    if rising_edge (sysclk(6)) then
      rdppud1 <= lrdppu;
      rdppud2 <= rdppud1;
    end if;
  end process;

  -- capture the write data cables from the various sources
  -- todo: ppu, regs
  -- ECS is done the easy way:
  iwecs <= wrecs4 (14 downto 0) & wrecs3 (14 downto 0) &
           wrecs2 (14 downto 0) & wrecs1 (14 downto 0);

  iwppu <= wrppu1 (11 downto 0)  & -- bits 59..48
           wrppu2 (8 downto 0)   & -- bits 47..39
           wrppu1 (14 downto 12) & -- bits 38..36
           wrppu3 (5 downto 0)   & -- bits 35..30
           wrppu2 (14 downto 9)  & -- bits 29..24
           wrppu4 (2 downto 0)   & -- bits 23..21
           wrppu3 (14 downto 6)  & -- bits 20..12
           wrppu4 (14 downto 3);   -- bits 11..0
           
  iwctrl <= (56 => wrctrl2(6), 35 => wrctrl2(5), 34 => wrctrl2(4),
             33 => wrctrl2(3), 32 => wrctrl2(2), 31 => wrctrl2(1),
             30 => wrctrl2(0),
             53 => wrctrl1(9), 52 => wrctrl1(8), 51 => wrctrl1(7),
             50 => wrctrl1(6), 49 => wrctrl1(5), 48 => wrctrl1(4),
             47 => wrctrl1(3), 46 => wrctrl1(2), 45 => wrctrl1(1),
             44 => wrctrl1(0), 43 => wrctrl1(10), 42 => wrctrl1(17),
             41 => wrctrl1(16), 40 => wrctrl1(15), 39 => wrctrl1(14),
             38 => wrctrl1(13), 37 => wrctrl1(12), 36 => wrctrl1(11),
             others => '0');
  
  -- Latch the correct write data.  PPU write data (from the write
  -- pyramid) is unusual because that it sent just once, at the time
  -- the address is sent to the stunt box.  That means before the
  -- memory gets the address and GO signals for that request.  It is
  -- latched in the write distributor and cleared when the write
  -- resume is sent back.  Other data sources send the data when it is
  -- expected, and the write distributor latched those buses on every
  -- minor cycle.  See block diagrams vol. 2, page 79 or 82.
  ppulatch : ireg
    port map (
      clr => iwresume,                  -- clear on write resume
      ibus => iwppu,
      obus => lwpdata);
      
  wlatch : process (sysclk(19))
  begin  -- process
    if rising_edge(sysclk(19)) then
      if lwppu = '1' then lwdata <= lwpdata;
      --elsif lwecs = '1' then lwdata <= iwecs;
      elsif lwctl = '1' then
        if lwreg = '1' then
          lwdata <= iwctrl (59 downto 36) & iwreg (35 downto 0);
        else lwdata <= iwctrl;
        end if;
      elsif lwreg = '1' then lwdata <= iwreg;
      else lwdata <= (others => '0');
      end if;
    end if;
  end process wlatch;
  twdata <= cpword (lwdata);
  
  -- 32 memory banks, 4k by 60 each
  mbank: for b in 0 to 31 generate
    cm : cmbank
      generic map (
        banknum => b)
      port map (
        go     => lgo,
        addr   => taddr,
        baddr  => bank,
        sysclk   => sysclk,
        reset  => reset,
        write  => lwrite,
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
    for i in cpword'range loop
      prdata(i) <= ttrdata(i) and rdppud2;
    end loop;  -- i
  end process trunks;

  -- Swizzle the read data for the output trunks
  -- chassis 1 cm to pyramid:
  -- 0..14 W04-90..904, 15..29 W05-90.904
  -- 30..37 W06-900..907, 38..44 W06-90..96, 45..52 W07-900..907,
  -- 53..59 W07-90..96
  rdpp1 (14 downto 0) <= prdata (14 downto 0);
  rdpp2 (14 downto 0) <= prdata (29 downto 15);
  rdpp3 (17 downto 10) <= prdata (37 downto 30);
  rdpp3 (6 downto 0) <= prdata (44 downto 38);
  rdpp4 (17 downto 10) <= prdata (52 downto 45);
  rdpp4 (6 downto 0) <= prdata (59 downto 53);

  -- generate read resume to PP
  c5full <= rdppud2 and sysclk(9);
  rresume <= rdppud2 and sysclk(9);
  iwresume <= lwppu and sysclk (5);
  wresume <= iwresume;
  
  -- generate write resume to PP
  -- TODO
  
  -- chassis 5 input register (A-E 41,42):
  -- 0..3 W02-904..907, 4 W02-900 5..14 W02-90..99
  -- 15..18 W01-904..907, 19 W01-900, 20..29 W01-90..99
  -- 30..37 W04-900..907, 38..44 W04-90..96, 
  -- 45..52 W03-900..907, 53..59 W03-90..96, 
  rdctrl1 (17 downto 14) <= rdata (3 downto 0);
  rdctrl1 (10) <= rdata (4);
  rdctrl1 (9 downto 0) <= rdata (14 downto 5);
  rdctrl2 (17 downto 14) <= rdata (18 downto 15);
  rdctrl2 (10) <= rdata (19);
  rdctrl2 (9 downto 0) <= rdata (29 downto 20);
  rdctrl3 (17 downto 10) <= rdata (37 downto 30);
  rdctrl3 (6 downto 0) <= rdata (44 downto 38);
  rdctrl4 (17 downto 10) <= rdata (52 downto 45);
  rdctrl4 (6 downto 0) <= rdata (59 downto 53);

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

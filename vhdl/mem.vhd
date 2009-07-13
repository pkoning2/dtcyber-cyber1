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

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

use work.sigs.all;

entity memarray is
  
  generic (
    idata : imem);
  port (
    addr   : in  ppword;                -- Memory address
    rdata  : out ppword;                -- read data
    rdatab : out ppword;                -- read data complemented
    wdata  : in  ppword;                -- write data (complemented)
    reset  : in  std_logic;             -- power-up reset
    strobe : in  std_logic;             -- read/write strobe
    write  : in  std_logic);            -- write request

end memarray;

architecture beh of memarray is
begin  -- beh

  -- purpose: read/write process
  -- type   : sequential
  -- inputs : read, write, addr, wdata
  -- outputs: rdata, rdatab
  rw: process (strobe, reset)
    variable areg : integer;              -- Address as an integer
    variable dreg : ppint;              -- data, as an integer
    variable mdata : ppmem;             -- memory array
    variable tdata : ppword;            -- copy of read data
  begin  -- process rw
    if reset = '1' then
      for i in 0 to idata'high loop
        mdata (i) := idata (i);
      end loop;  -- i
    elsif strobe'event and strobe = '1' then  -- rising clock edge
      areg := TO_INTEGER (addr);
      if write = '1' then
        if idata'high = 1 then
          mdata (areg) := TO_INTEGER (not (wdata));
        end if;
      else
        tdata := TO_UNSIGNED (mdata (areg), 12);
        rdata <= tdata;
        rdatab <= not (tdata);
      end if;
    end if;
  end process rw;
end beh;

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity mem is
  generic (
    idata : imem := (0,0));
  
  port (
    reset  : in  std_logic;              -- power-up reset
    p22 : in std_logic;                  -- addr(0)
    p20 : in std_logic;                  -- addr(1)
    p8 : in std_logic;                   -- addr(2)
    p10 : in std_logic;                  -- addr(3)
    p107   : in std_logic;               -- addr(4)
    p109   : in std_logic;               -- addr(5)
    p113  : in std_logic;                -- addr(6)
    p111  : in std_logic;                -- addr(7)
    p12 : in std_logic;                  -- addr(8)
    p14 : in std_logic;                  -- addr(9)
    p18 : in std_logic;                  -- addr(10)
    p16 : in std_logic;                  -- addr(11)
    p129  : in std_logic;                -- wdata(0)
    p105   : in std_logic;               -- wdata(1)
    p6 : in std_logic;                   -- wdata(2)
    p26 : in std_logic;                  -- wdata(3)
    p127  : in std_logic;                -- wdata(4)
    p101   : in std_logic;               -- wdata(5)
    p28 : in std_logic;                  -- wdata(6)
    p4 : in std_logic;                   -- wdata(7)
    p103   : in std_logic;               -- wdata(8)
    p125  : in std_logic;                -- wdata(9)
    p30 : in std_logic;                  -- wdata(10)
    p2 : in std_logic;                   -- wdata(11)
    p1 : out std_logic;                  -- rdata(0)
    p3 : out std_logic;                  -- rdatab(0)
    p27 : out std_logic;                 -- rdata(1)
    p29 : out std_logic;                 -- rdatab(1)
    p128  : out std_logic;               -- rdata(2)
    p130  : out std_logic;               -- rdatab(2)
    p102   : out std_logic;              -- rdata(3)
    p104   : out std_logic;              -- rdatab(3)
    p5 : out std_logic;                  -- rdata(4)
    p7 : out std_logic;                  -- rdatab(4)
    p23 : out std_logic;                 -- rdata(5)
    p25 : out std_logic;                 -- rdatab(5)
    p106   : out std_logic;              -- rdata(6)
    p108   : out std_logic;              -- rdatab(6)
    p124  : out std_logic;               -- rdata(7)
    p126  : out std_logic;               -- rdatab(7)
    p19 : out std_logic;                 -- rdata(8)
    p21 : out std_logic;                 -- rdatab(8)
    p9 : out std_logic;                  -- rdata(9)
    p11 : out std_logic;                 -- rdatab(9)
    p110  : out std_logic;               -- rdata(10)
    p112  : out std_logic;               -- rdatab(10)
    p120  : out std_logic;               -- rdata(11)
    p122  : out std_logic;               -- rdatab(11)
    p24 : in std_logic;                  -- quadrant 11 (unused)
    p119  : in std_logic;                -- quadrant 10 (unused)
    p117  : in std_logic;                -- quadrant 01 (unused)
    p115  : in std_logic;                -- quadrant 00 (unused)
    p121  : in std_logic;                -- not write
    p123  : in std_logic);               -- not read
end mem;

architecture beh of mem is
  component memarray
    generic (
      idata : imem);
    port (
      addr   : in  ppword;                -- Memory address
      rdata  : out ppword;                -- read data
      rdatab : out ppword;                -- read data complemented
      wdata  : in  ppword;                -- write data (complemented)
      reset  : in  std_logic;             -- power-up reset
      strobe : in  std_logic;             -- read/write strobe
      write  : in  std_logic);            -- write operation
  end component;
  signal s, w : std_logic;
begin  -- beh

  s <= not (p123) or not (p121);
  w <= not (p121);
  ar : memarray generic map (
    idata => idata)
    port map (
    addr(0) => p22,
    addr(1) => p20,
    addr(2) => p8,
    addr(3) => p10,
    addr(4) => p107,
    addr(5) => p109,
    addr(6) => p113,
    addr(7) => p111,
    addr(8) => p12,
    addr(9) => p14,
    addr(10) => p18,
    addr(11) => p16,
    wdata(0) => p129,
    wdata(1) => p105,
    wdata(2) => p6,
    wdata(3) => p26,
    wdata(4) => p127,
    wdata(5) => p101,
    wdata(6) => p28,
    wdata(7) => p4,
    wdata(8) => p103,
    wdata(9) => p125,
    wdata(10) => p30,
    wdata(11) => p2,
    rdata(0) => p1,
    rdata(1) => p27,
    rdata(2) => p128,
    rdata(3) => p102,
    rdata(4) => p5,
    rdata(5) => p23,
    rdata(6) => p106,
    rdata(7) => p124,
    rdata(8) => p19,
    rdata(9) => p9,
    rdata(10) => p110,
    rdata(11) => p120,
    rdatab(0) => p3,
    rdatab(1) => p29,
    rdatab(2) => p130,
    rdatab(3) => p104,
    rdatab(4) => p7,
    rdatab(5) => p25,
    rdatab(6) => p108,
    rdatab(7) => p126,
    rdatab(8) => p21,
    rdatab(9) => p11,
    rdatab(10) => p112,
    rdatab(11) => p122,
    reset => reset,
    write => w,
    strobe => s);

end beh;

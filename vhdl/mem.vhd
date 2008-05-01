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

use work.sigs.all;

entity memarray is
  
  port (
    addr   : in  ppword;                -- Memory address
    rdata  : out ppword;                -- read data
    rdatab : out ppword;                -- read data complemented
    wdata  : in  ppword;                -- write data (complemented)
    read   : in  std_logic;             -- read strobe
    write  : in  std_logic);            -- write strobe

end memarray;

architecture beh of memarray is
begin  -- beh

  -- purpose: read/write process
  -- type   : sequential
  -- inputs : read, write, addr, wdata
  -- outputs: rdata, rdatab
  rw: process (read, write)
    variable areg : integer;              -- Address as an integer
    variable mdata : ppmem;               -- Memory data
  begin  -- process rw
    if read'event and read = '1' then  -- rising clock edge
      areg := vec_to_int (addr);
      rdata <= mdata (areg);
      rdatab <= not (mdata (areg));
    elsif write'event and write = '1' then
      areg := vec_to_int (addr);
      mdata (areg) := not (wdata);
    end if;
  end process rw;
end beh;

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity mem is
  
  port (
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
    p121  : in std_logic;                -- write
    p123  : in std_logic);               -- read
end mem;

architecture beh of mem is
  component memarray
    port (
      addr   : in  ppword;                -- Memory address
      rdata  : out ppword;                -- read data
      rdatab : out ppword;                -- read data complemented
      wdata  : in  ppword;                -- write data (complemented)
      read   : in  std_logic;             -- read strobe
      write  : in  std_logic);            -- write strobe
  end component;
begin  -- beh

  ar : memarray port map (
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
    write => p121,
    read => p123);

end beh;

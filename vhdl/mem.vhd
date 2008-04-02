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
      areg := pp_to_int (addr);
      rdata <= mdata (areg);
      rdatab <= not (mdata (areg));
    elsif write'event and write = '1' then
      areg := pp_to_int (addr);
      mdata (areg) := not (wdata);
    end if;
  end process rw;
end beh;

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity mem is
  
  port (
    p122 : in std_logic;                -- addr(0)
    p120 : in std_logic;                -- addr(1)
    p108 : in std_logic;                -- addr(2)
    p110 : in std_logic;                -- addr(3)
    p7   : in std_logic;                -- addr(4)
    p9   : in std_logic;                -- addr(5)
    p13  : in std_logic;                -- addr(6)
    p11  : in std_logic;                -- addr(7)
    p112 : in std_logic;                -- addr(8)
    p114 : in std_logic;                -- addr(9)
    p118 : in std_logic;                -- addr(10)
    p116 : in std_logic;                -- addr(11)
    p29  : in std_logic;                -- wdata(0)
    p5   : in std_logic;                -- wdata(1)
    p106 : in std_logic;                -- wdata(2)
    p126 : in std_logic;                -- wdata(3)
    p27  : in std_logic;                -- wdata(4)
    p1   : in std_logic;                -- wdata(5)
    p128 : in std_logic;                -- wdata(6)
    p104 : in std_logic;                -- wdata(7)
    p3   : in std_logic;                -- wdata(8)
    p25  : in std_logic;                -- wdata(9)
    p130 : in std_logic;                -- wdata(10)
    p102 : in std_logic;                -- wdata(11)
    p101 : out std_logic;               -- rdata(0)
    p103 : out std_logic;               -- rdatab(0)
    p127 : out std_logic;               -- rdata(1)
    p129 : out std_logic;               -- rdatab(1)
    p28  : out std_logic;               -- rdata(2)
    p30  : out std_logic;               -- rdatab(2)
    p2   : out std_logic;               -- rdata(3)
    p4   : out std_logic;               -- rdatab(3)
    p105 : out std_logic;               -- rdata(4)
    p107 : out std_logic;               -- rdatab(4)
    p123 : out std_logic;               -- rdata(5)
    p125 : out std_logic;               -- rdatab(5)
    p6   : out std_logic;               -- rdata(6)
    p8   : out std_logic;               -- rdatab(6)
    p24  : out std_logic;               -- rdata(7)
    p26  : out std_logic;               -- rdatab(7)
    p119 : out std_logic;               -- rdata(8)
    p121 : out std_logic;               -- rdatab(8)
    p109 : out std_logic;               -- rdata(9)
    p111 : out std_logic;               -- rdatab(9)
    p10  : out std_logic;               -- rdata(10)
    p12  : out std_logic;               -- rdatab(10)
    p20  : out std_logic;               -- rdata(11)
    p22  : out std_logic;               -- rdatab(11)
    p124 : in std_logic;                -- quadrant 11 (unused)
    p19  : in std_logic;                -- quadrant 10 (unused)
    p17  : in std_logic;                -- quadrant 01 (unused)
    p15  : in std_logic;                -- quadrant 00 (unused)
    p21  : in std_logic;                -- write
    p23  : in std_logic);               -- read
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
    addr(0) => p122,
    addr(1) => p120,
    addr(2) => p108,
    addr(3) => p110,
    addr(4) => p7,
    addr(5) => p9,
    addr(6) => p13,
    addr(7) => p11,
    addr(8) => p112,
    addr(9) => p114,
    addr(10) => p118,
    addr(11) => p116,
    wdata(0) => p29,
    wdata(1) => p5,
    wdata(2) => p106,
    wdata(3) => p126,
    wdata(4) => p27,
    wdata(5) => p1,
    wdata(6) => p128,
    wdata(7) => p104,
    wdata(8) => p3,
    wdata(9) => p25,
    wdata(10) => p130,
    wdata(11) => p102,
    rdata(0) => p101,
    rdata(1) => p127,
    rdata(2) => p28,
    rdata(3) => p2,
    rdata(4) => p105,
    rdata(5) => p123,
    rdata(6) => p6,
    rdata(7) => p24,
    rdata(8) => p119,
    rdata(9) => p109,
    rdata(10) => p10,
    rdata(11) => p20,
    rdatab(0) => p103,
    rdatab(1) => p129,
    rdatab(2) => p30,
    rdatab(3) => p4,
    rdatab(4) => p107,
    rdatab(5) => p125,
    rdatab(6) => p8,
    rdatab(7) => p26,
    rdatab(8) => p121,
    rdatab(9) => p111,
    rdatab(10) => p12,
    rdatab(11) => p22,
    write => p21,
    read => p21);

end beh;

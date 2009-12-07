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

entity memarray12 is
  
  generic (
    idata : ippmem);
  port (
    addr   : in  ppword;                -- Memory address
    rdata  : out ppword;                -- read data
    wdata  : in  ppword;                -- write data (complemented)
    reset  : in  logicsig;             -- power-up reset
    strobe : in  logicsig;             -- read/write strobe
    write  : in  logicsig);            -- write request

end memarray12;

architecture beh of memarray12 is
begin  -- beh

  -- purpose: read/write process
  -- type   : sequential
  -- inputs : read, write, addr, wdata
  -- outputs: rdata
  rw: process (strobe, reset)
    variable areg : integer;              -- Address as an integer
    variable mdata : ppmem;             -- memory array
  begin  -- process rw
    if reset = '1' then
      for i in 0 to idata'high loop
        mdata (i) := TO_UNSIGNED (idata (i), 12);
      end loop;  -- i
    elsif strobe'event and strobe = '1' then  -- rising clock edge
      areg := TO_INTEGER (addr);
      if write = '1' then
        mdata (areg) := not (wdata);
      else
        rdata <= mdata (areg);
      end if;
    end if;
  end process rw;
end beh;

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

use work.sigs.all;

entity memarray60 is
  
--  generic (
--    idata : icpmem);
  port (
    addr   : in  ppword;                -- Memory address
    rdata  : out cpword;                -- read data
    wdata  : in  cpword;                -- write data
    reset  : in  logicsig;             -- power-up reset
    strobe : in  logicsig;             -- read/write strobe
    write  : in  logicsig);            -- write request

end memarray60;

architecture beh of memarray60 is
begin  -- beh

  -- purpose: read/write process
  -- type   : sequential
  -- inputs : read, write, addr, wdata
  -- outputs: rdata
  rw: process (strobe, reset)
    variable areg : integer;              -- Address as an integer
    variable mdata : cpmem;             -- memory array
  begin  -- process rw
--    if reset = '1' then
--      for i in 0 to idata'high loop
--        mdata (i) := TO_UNSIGNED (idata (i), 60);
--      end loop;  -- i
--    elsif
  if strobe'event and strobe = '1' then  -- rising clock edge
      areg := TO_INTEGER (addr);
      if write = '1' then
        mdata (areg) := wdata;
      else
        rdata <= mdata (areg);
      end if;
    end if;
  end process rw;
end beh;

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity mem is
  generic (
    idata : ippmem := (0,0));
  
  port (
    reset  : in  logicsig;              -- power-up reset
    p22 : in logicsig;                  -- addr(0)
    p20 : in logicsig;                  -- addr(1)
    p8 : in logicsig;                   -- addr(2)
    p10 : in logicsig;                  -- addr(3)
    p107   : in logicsig;               -- addr(4)
    p109   : in logicsig;               -- addr(5)
    p113  : in logicsig;                -- addr(6)
    p111  : in logicsig;                -- addr(7)
    p12 : in logicsig;                  -- addr(8)
    p14 : in logicsig;                  -- addr(9)
    p18 : in logicsig;                  -- addr(10)
    p16 : in logicsig;                  -- addr(11)
    p129  : in logicsig;                -- wdata(0)
    p105   : in logicsig;               -- wdata(1)
    p6 : in logicsig;                   -- wdata(2)
    p26 : in logicsig;                  -- wdata(3)
    p127  : in logicsig;                -- wdata(4)
    p101   : in logicsig;               -- wdata(5)
    p28 : in logicsig;                  -- wdata(6)
    p4 : in logicsig;                   -- wdata(7)
    p103   : in logicsig;               -- wdata(8)
    p125  : in logicsig;                -- wdata(9)
    p30 : in logicsig;                  -- wdata(10)
    p2 : in logicsig;                   -- wdata(11)
    p1 : out logicsig;                  -- rdata(0)
    p3 : out logicsig;                  -- rdatab(0)
    p27 : out logicsig;                 -- rdata(1)
    p29 : out logicsig;                 -- rdatab(1)
    p128  : out logicsig;               -- rdata(2)
    p130  : out logicsig;               -- rdatab(2)
    p102   : out logicsig;              -- rdata(3)
    p104   : out logicsig;              -- rdatab(3)
    p5 : out logicsig;                  -- rdata(4)
    p7 : out logicsig;                  -- rdatab(4)
    p23 : out logicsig;                 -- rdata(5)
    p25 : out logicsig;                 -- rdatab(5)
    p106   : out logicsig;              -- rdata(6)
    p108   : out logicsig;              -- rdatab(6)
    p124  : out logicsig;               -- rdata(7)
    p126  : out logicsig;               -- rdatab(7)
    p19 : out logicsig;                 -- rdata(8)
    p21 : out logicsig;                 -- rdatab(8)
    p9 : out logicsig;                  -- rdata(9)
    p11 : out logicsig;                 -- rdatab(9)
    p110  : out logicsig;               -- rdata(10)
    p112  : out logicsig;               -- rdatab(10)
    p120  : out logicsig;               -- rdata(11)
    p122  : out logicsig;               -- rdatab(11)
    p24 : in logicsig;                  -- quadrant 11 (unused)
    p119  : in logicsig;                -- quadrant 10 (unused)
    p117  : in logicsig;                -- quadrant 01 (unused)
    p115  : in logicsig;                -- quadrant 00 (unused)
    p121  : in logicsig;                -- not write
    p123  : in logicsig);               -- not read
end mem;

architecture beh of mem is
  component memarray12
    generic (
      idata : ippmem);
    port (
      addr   : in  ppword;                -- Memory address
      rdata  : out ppword;                -- read data
      wdata  : in  ppword;                -- write data (complemented)
      reset  : in  logicsig;             -- power-up reset
      strobe : in  logicsig;             -- read/write strobe
      write  : in  logicsig);            -- write operation
  end component;
  signal s, w : logicsig;
  signal tdata, tdatab : ppword;                -- copy of read data
begin  -- beh

  s <= not (p123) or not (p121);
  w <= not (p121);
  ar : memarray12 generic map (
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
    rdata => tdata,
    reset => reset,
    write => w,
    strobe => s);
  tdatab <= tdata;
  p1   <= tdata(0);
  p27  <= tdata(1);
  p128 <= tdata(2);
  p102 <= tdata(3);
  p5   <= tdata(4);
  p23  <= tdata(5);
  p106 <= tdata(6);
  p124 <= tdata(7);
  p19  <= tdata(8);
  p9   <= tdata(9);
  p110 <= tdata(10);
  p120 <= tdata(11);
  p3   <= tdatab(0);
  p29  <= tdatab(1);
  p130 <= tdatab(2);
  p104 <= tdatab(3);
  p7   <= tdatab(4);
  p25  <= tdatab(5);
  p108 <= tdatab(6);
  p126 <= tdatab(7);
  p21  <= tdatab(8);
  p11  <= tdatab(9);
  p112 <= tdatab(10);
  p122 <= tdatab(11);

end beh;

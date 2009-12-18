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
-- Model of PP memory
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.numeric_bit.all;

use work.sigs.all;

entity mem is
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
  signal s, w : logicsig;
  signal areg : ppword;                 -- copy of address
  signal wreg : unsigned(15 downto 0);  -- copy of write data
  signal tdata : unsigned(15 downto 0); -- copy of read data
begin  -- beh

  s <= not (p123) or not (p121);
  w <= not (p121);
  areg <= (p16, p18, p14, p12, p111, p113, p109, p107, p10, p8, p20, p22);
  wreg <= "0000" & not ((p2, p30, p125, p103, p4, p28, p101, p127, p26, p6, p105, p129));
  arl : memarray generic map (
      abits => 12,
      dbits => 8)
    port map (
      addr_a => areg,
      wdata_a => wreg(7 downto 0),
      rdata_a => tdata(7 downto 0),
      reset => reset,
      write_a => w,
      ena_a => s,
      clk_a => s);
  arh : memarray generic map (
      abits => 12,
      dbits => 8)
    port map (
      addr_a => areg,
      wdata_a => wreg(15 downto 8),
      rdata_a => tdata(15 downto 8),
      reset => reset,
      write_a => w,
      ena_a => s,
      clk_a => s);
  (p120, p110, p9, p19, p124, p106, p23, p5, p102, p128, p27, p1) <= tdata(11 downto 0);
  (p122, p112, p11, p21, p126, p108, p25, p7, p104, p130, p29, p3) <= tdata(11 downto 0);

end beh;

-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2009 by Paul Koning
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
use IEEE.numeric_bit.all;

-- Common definitions for CDC 6600 model

-- signal definitions

package sigs is

  subtype coaxsig is bit;                 -- signal on coax
  subtype logicsig is bit;                -- logic or tp signal
  constant t : time := 5 ns;            -- basic stage delay
  constant tp : time := 10 ns;          -- twisted pair wire delay
  constant tc : time := 25 ns;          -- coax delay (including transistors)
  type coaxbus is array (natural range <>) of coaxsig;  -- coax cable, any width
  subtype coaxsigs is coaxbus (0 to 18);    -- CDC standard coax cable
  constant idlecoax : coaxsigs := (others => '0');
  type tpcable is array (0 to 23) of logicsig;    -- CDC standard tp cable
  constant idletp : tpcable := (others => '0');
  subtype ppword is UNSIGNED (11 downto 0);  -- PPU word (12 bits)
  subtype ppint is integer range 0 to 4095;  -- PPU word, as an integer
  type ppmem is array (0 to 4095) of ppword;  -- standard 4kx12 memory array
  subtype bankaddr is UNSIGNED (4 downto 0);  -- Bank address (5 bits)
  subtype cpword is UNSIGNED (59 downto 0);  -- CPU word (60 bits)
  type cpmem is array (0 to 4095) of cpword;  -- 4kx60 memory array
  type misc is ('x', 'y');  -- used for non-logic pins
  subtype analog is UNSIGNED (2 downto 0);    -- 6612 character drawing signal
  
  procedure dtconn (
    constant chnum : in    integer;     -- Channel number for this synchronizer
    incable        : inout coaxsigs;    -- Input cable
    outcable       : inout coaxsigs);   -- Output cable
  attribute foreign of dtconn : procedure is "VHPIDIRECT dtconn";

  procedure dtmain ;
  attribute foreign of dtmain : procedure is "VHPIDIRECT dtmain";

end sigs;

package body sigs is

  -- purpose: Dummy body
  procedure dtconn (
    constant chnum : in    integer;
    incable        : inout coaxsigs;
    outcable       : inout coaxsigs) is
  begin  -- dtconn
    assert false severity failure;
  end dtconn;
  
  -- purpose: Dummy body
  procedure dtmain  is
  begin  -- dtmain
    assert false severity failure;
  end dtmain;
  
end sigs;

use work.sigs.all;
entity inv is
  
  port (
    a  : in  logicsig;                       -- input
    y  : out logicsig);                      -- output

end inv;

architecture bool of inv is
begin  -- bool
  inv: process (a)
  begin  -- process inv
    y <= not (a) after t;
  end process inv;
end bool;

-- inv2 is two inverters in a row, used by the 6600 for fanout
-- (or perhaps for delay)

use work.sigs.all;
entity inv2 is
  
  port (
    a  : in  logicsig;                       -- input
    y, y2  : out logicsig);                      -- output

end inv2;

architecture bool of inv2 is
begin  -- bool
  inv: process (a)
  begin  -- process inv
    y <= not (a) after t;
    y2 <= a after 2 * t;
  end process inv;
end bool;

-- The CDC docs talk about the gates as "NOR", meaning "NOT OR".
-- In standard terminology, that's "NAND" (AND NOT).  So we'll
-- write the that way here.

use work.sigs.all;
entity g2 is
  
  port (
    a, b : in  logicsig;                     -- inputs
    y, y2    : out logicsig);                    -- output

end g2;

architecture bool of g2 is
begin  -- bool
  g2: process (a, b)
    variable t1 : logicsig;
  begin  -- process g2
    t1 := a and b;
    y <= not (t1) after t;
    y2 <= t1 after t * 2;
  end process g2;
end bool;

use work.sigs.all;
entity g3 is
  
  port (
    a, b, c : in  logicsig;                  -- inputs
    y, y2    : out logicsig);                    -- output

end g3;

architecture bool of g3 is
begin  -- bool
  g3: process (a, b, c)
    variable t1 : logicsig;
  begin  -- process g3
    t1 := a and b and c;
    y <= not (t1) after t;
    y2 <= t1 after t * 2;
  end process g3;
end bool;

use work.sigs.all;
entity g4 is
  
  port (
    a, b, c, d : in  logicsig;               -- inputs
    y, y2    : out logicsig);                    -- output

end g4;

architecture bool of g4 is
begin  -- bool
  g4: process (a, b, c, d)
    variable t1 : logicsig;
  begin  -- process g4
    t1 := a and b and c and d;
    y <= not (t1) after t;
    y2 <= t1 after t * 2;
  end process g4;
end bool;

use work.sigs.all;
entity g5 is
  
  port (
    a, b, c, d, e : in  logicsig;            -- inputs
    y, y2    : out logicsig);                    -- output

end g5;

architecture bool of g5 is
begin  -- bool
  g5: process (a, b, c, d, e)
    variable t1 : logicsig;
  begin  -- process g5
    t1 := a and b and c and d and e;
    y <= not (t1) after t;
    y2 <= t1 after t * 2;
  end process g5;
end bool;

use work.sigs.all;
entity g6 is
  
  port (
    a, b, c, d, e, f : in  logicsig;         -- inputs
    y, y2    : out logicsig);                    -- output

end g6;

architecture bool of g6 is
  signal ai, bi, ci, di, ei, fi : logicsig;
  signal yi, y2i : logicsig;
begin  -- bool
  g6: process (a, b, c, d, e, f)
    variable t1 : logicsig;
  begin  -- process g6
    t1 := a and b and c and d and e and f;
    y <= not (t1) after t;
    y2 <= t1 after t * 2;
  end process g6;
end bool;

use work.sigs.all;
entity cxdriver is
  
  port (
    a : in  logicsig;                        -- source
    y : out coaxsig);                         -- destination

end cxdriver;

architecture bool of cxdriver is
begin  -- bool
  inv: process (a)
  begin  -- process inv
    y <= not (a) after tc;
  end process inv;
end bool;

use work.sigs.all;
entity cxdriver5 is
  
  port (
    a, a2, a3, a4, a5 : in  logicsig;        -- sources
    y : out coaxsig);                         -- destination

end cxdriver5;

architecture bool of cxdriver5 is
begin  -- bool
  g5: process (a, a2, a3, a4, a5)
    variable t1 : logicsig;
  begin  -- process g5
    t1 := a and a2 and a3 and a4 and a5;
    y <= not (t1) after tc;
  end process g5;
end bool;

use work.sigs.all;
entity cxreceiver is
  
  port (
    a : in  coaxsig;                    -- source
    y : out logicsig);                 -- destination

end cxreceiver;

architecture bool of cxreceiver is
  signal ai : logicsig;
begin  -- bool
  y <= not (a);                        -- coax is positive logic...
end bool;

use work.sigs.all;
entity rsflop is
  port (
    s, r  : in  logicsig;                    -- set, reset
    q, qb : out logicsig);                   -- q and q.bar

end rsflop;

architecture beh of rsflop is
begin  -- beh
  rsflop: process (r, s)
    variable qi : logicsig;
  begin  -- process rsflop
    if r = '0' then
      qi := '0';
    elsif s = '0' then
      qi := '1';
    end if;
    q <= qi after t;
    qb <= not (qi) after t;
  end process rsflop;
end beh;


use work.sigs.all;
entity rs4flop is
  port (
    s, r  : in  logicsig;                    -- set, reset
    s2, s3, s4  : in  logicsig;  -- extra set
    q, qb : out logicsig);                   -- q and q.bar

end rs4flop;

architecture beh of rs4flop is
  component rsflop
    port (
      s, r  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal si : logicsig := '0';
begin  -- beh
  si <= s and s2 and s3 and s4;
  u1 : rsflop port map (
    s  => si,
    r  => r,
    q  => q,
    qb => qb);

end beh;


use work.sigs.all;
entity r4s4flop is
  port (
    s, r  : in  logicsig;                    -- set, reset
    s2, s3, s4, r2, r3, r4  : in  logicsig;  -- extra set, reset
    q, qb : out logicsig);                   -- q and q.bar

end r4s4flop;

architecture beh of r4s4flop is
  component rsflop
    port (
      s, r  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal ri, si, qi : logicsig := '0';
begin  -- beh
  ri <= r and r2 and r3 and r4;
  si <= s and s2 and s3 and s4;
  u1 : rsflop port map (
    s  => si,
    r  => ri,
    q  => q,
    qb => qb);

end beh;


use work.sigs.all;
entity rs2flop is
  port (
    s, r  : in  logicsig;                    -- set, reset
    s2  : in  logicsig;                 -- extra set
    q, qb : out logicsig);                   -- q and q.bar

end rs2flop;

architecture beh of rs2flop is
  component rsflop
    port (
      s, r  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal si : logicsig;
begin  -- beh
  si <= s and s2;
  u1 : rsflop port map (
    s  => si,
    r  => r,
    q  => q,
    qb => qb);

end beh;


use work.sigs.all;
entity r2sflop is
  port (
    s, r  : in  logicsig;                    -- set, reset
    r2  : in  logicsig;                -- extra reset
    q, qb : out logicsig);                   -- q and q.bar

end r2sflop;

architecture beh of r2sflop is
  component rsflop
    port (
      s, r  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal ri : logicsig := '0';
begin  -- beh
  ri <= r and r2;
  u1 : rsflop port map (
    s  => s,
    r  => ri,
    q  => q,
    qb => qb);

end beh;


use work.sigs.all;
entity latch is
  
  port (
    d, clk : in  logicsig;                   -- data (set), clock
    q, qb  : out logicsig);                  -- q and q.bar

end latch;

architecture beh of latch is
  signal clki : logicsig;
begin  -- beh
  clki <= clk after t * 2;
  latch: process (clki, d)
    variable qi : logicsig;
  begin  -- process level sensitive latch
    if clki = '1' then  -- clock (enable) asserted
      qi := d;
    end if;
    q <= qi after t;
    qb <= not (qi) after t;
  end process latch;
end beh;


use work.sigs.all;
entity latch2 is
  
  port (
    d, clk, clk2 : in  logicsig;                   -- data (set), clocks
    q, qb  : out logicsig);                  -- q and q.bar

end latch2;

architecture beh of latch2 is
  component latch
    port (
      d, clk : in  logicsig;                 -- data (set), clock
      q, qb  : out logicsig);                -- q and q.bar
  end component;
  signal clki : logicsig;
begin  -- beh
  clki <= clk and clk2;
  u1 : latch port map (
    d   => d,
    clk => clki,
    q   => q,
    qb  => qb);

end beh;


use work.sigs.all;
entity latchd2 is
  
  port (
    d, d2, clk : in  logicsig;                   -- data (set), clock
    q, qb  : out logicsig);                  -- q and q.bar

end latchd2;

architecture beh of latchd2 is
  component latch
    port (
      d, clk : in  logicsig;                 -- data (set), clock
      q, qb  : out logicsig);                -- q and q.bar
  end component;
  signal di : logicsig;
begin  -- beh
  di <= d and d2;
  u1 : latch port map (
    d   => di,
    clk => clk,
    q   => q,
    qb  => qb);

end beh;


use work.sigs.all;
entity latchd4 is
  
  port (
    d, d2, e, e2, clk : in  logicsig;                   -- data (set), clock
    q, qb  : out logicsig);                  -- q and q.bar

end latchd4;

architecture beh of latchd4 is
  component latch
    port (
      d, clk : in  logicsig;                 -- data (set), clock
      q, qb  : out logicsig);                -- q and q.bar
  end component;
  signal di : logicsig;
begin  -- beh
  di <= (d and e) or (d2 and e2);
  u1 : latch port map (
    d   => di,
    clk => clk,
    q   => q,
    qb  => qb);

end beh;


use work.sigs.all;
entity latchr is
  
  port (
    d, clk : in  logicsig;                   -- data (set), clock
    r      : in  logicsig;                   -- reset
    q, qb  : out logicsig);                  -- q and q.bar

end latchr;

architecture beh of latchr is
  signal clki : logicsig;
  signal qi : logicsig;
begin  -- beh
  clki <= clk after t * 2;
  latch: process (clki, d, r)
    variable qi : logicsig;
  begin  -- process level sensitive latch with reset
    if r = '0' then
      qi := '0';
    elsif clki = '1' then  -- clock (enable) asserted
      qi := d;
    end if;
    q <= qi after t;
    qb <= not (qi) after t;
  end process latch;
end beh;


use work.sigs.all;
  
entity wire is
  
  generic (
    length : integer);                -- length in inches

  port (
    i  : in  logicsig;                 -- input
    o  : out logicsig);                -- output

end wire;

architecture beh of wire is
  constant feet : real := real (length) / 12.0;
  constant idelay : time := feet * 1.3 ns;
begin  -- beh

  o <= transport i after idelay;

end beh;

-- Memory array, loosely based on the Xilinx Virtex 6 embedded memory.
-- Various sizes are available, see the book.  For that matter, this
-- behavior model also allows sizes that don't actually exist in the FPGA...

library IEEE;
use IEEE.numeric_bit.all;
use work.sigs.all;

entity memarray is
  
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

end memarray;

architecture beh of memarray is
begin  -- beh

  rw: process (clk_a, clk_b, reset)
    constant maxaddr : integer := 2 ** abits - 1;  -- max address
    subtype mdata_t is unsigned (dbits - 1 downto 0);
    type marray_t is array (0 to maxaddr) of mdata_t;
    variable areg : integer;              -- Address as an integer
    variable mdata : marray_t;
  begin  -- process rw
    areg := 0;                          -- dummy init, it's not a latch
    if clk_a'event and clk_a = '1' then  -- rising clock edge, port A
      if ena_a = '1' then
        areg := TO_INTEGER (addr_a);
        if write_a = '1' then
          mdata (areg) := wdata_a;
        else
          rdata_a <= mdata (areg);
        end if;
      end if;
    end if;
    if clk_b'event and clk_b = '1' then  -- rising clock edge, port B
      if ena_b = '1' then
        areg := TO_INTEGER (addr_b);
        if write_b = '1' then
          mdata (areg) := wdata_b;
        else
          rdata_b <= mdata (areg);
        end if;
      end if;
    end if;
  end process rw;
end beh;

use work.sigs.all;

entity ireg is
  
  port (
    clr : in bit;                       -- clear pulse
    ibus : in coaxbus;                    -- input bus
    obus : out coaxbus);                  -- output bus

end ireg;

architecture beh of ireg is
begin  -- ireg
  -- purpose: input latch
  ilatch: process (ibus, clr)
  begin  -- process ilatch
    if clr = '1' then
      for i in obus'range loop
        obus(i) <= '0';
      end loop;  -- i
      --obus <= (others => '0');
    end if;
    for i in ibus'range loop
      if ibus(i) = '1' then
        obus(i) <= '1';
      end if;
    end loop;
  end process ilatch;
end beh;

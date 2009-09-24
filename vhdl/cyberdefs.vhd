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

-- Common definitions for CDC 6600 model

-- signal definitions

package sigs is

  constant v0 : std_logic := '1';             -- low voltage (logic 1)
  constant v1 : std_logic := '0';             -- high voltage (logic 0)
  constant t : time := 5 ns;            -- basic stage delay
  constant tp : time := 10 ns;          -- twisted pair wire delay
  constant tc : time := 25 ns;          -- coax delay (including transistors)
  subtype coaxsig is std_logic range 'U' to '1';  -- signal on coax
  subtype coaxsigs is std_logic_vector (1 to 19);    -- CDC standard coax cable
  subtype ppword is UNSIGNED (11 downto 0);  -- PPU word (12 bits)
  subtype ppint is integer range 0 to 4095;  -- PPU word, as an integer
  type ppmem is array (0 to 4095) of ppint;  -- standard 4kx12 memory array
  type imem  is array (natural range <>) of ppint;  -- initial data for ppmem
  subtype misc is std_logic range '0' to '1';  -- used for non-logic pins
  subtype analog is UNSIGNED (2 downto 0);    -- 6612 character drawing signal
  
  procedure dtsynchro (
    constant chnum : in    integer;     -- Channel number for this synchronizer
    incable        : inout coaxsigs;    -- Input cable
    outcable       : inout coaxsigs);   -- Output cable
  attribute foreign of dtsynchro : procedure is "VHPIDIRECT dtsynchro";

  procedure dtmain ;
  attribute foreign of dtmain : procedure is "VHPIDIRECT dtmain";

end sigs;

package body sigs is

  -- purpose: Dummy body
  procedure dtsynchro (
    constant chnum : in    integer;
    incable        : inout coaxsigs;
    outcable       : inout coaxsigs) is
  begin  -- dtsynchro
    assert false severity failure;
  end dtsynchro;
  
  -- purpose: Dummy body
  procedure dtmain  is
  begin  -- dtmain
    assert false severity failure;
  end dtmain;
  
end sigs;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity inv is
  
  port (
    a  : in  std_logic;                       -- input
    y  : out std_logic);                      -- output

end inv;

architecture bool of inv is
  signal ii : std_logic;
  signal oi : std_logic;
begin  -- bool
  ii <= '1' when a = 'U' else a;
  oi <= not (ii) after t;
  y <= oi;

end bool;

-- inv2 is two inverters in a row, used by the 6600 for fanout
-- (or perhaps for delay)

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity inv2 is
  
  port (
    a  : in  std_logic;                       -- input
    y, y2  : out std_logic);                      -- output

end inv2;

architecture bool of inv2 is
  signal ii : std_logic;
  signal oi, oi2 : std_logic;
begin  -- bool
  ii <= '1' when a = 'U' else a;
  oi <= not (ii) after t;
  oi2 <= ii after 2 * t;
  y <= oi;
  y2 <= oi2;
  
end bool;

-- The CDC docs talk about the gates as "NOR", meaning "NOT OR".
-- In standard terminology, that's "NAND" (AND NOT).  So we'll
-- write the that way here.

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g2 is
  
  port (
    a, b : in  std_logic;                     -- inputs
    y, y2    : out std_logic);                    -- output

end g2;

architecture bool of g2 is
  signal ai, bi : std_logic;
  signal yi, y2i : std_logic;
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  yi <= not (ai and bi) after t;
  y2i <= (ai and bi) after 2 * t;
  y <= yi;
  y2 <= y2i;
  
end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g3 is
  
  port (
    a, b, c : in  std_logic;                  -- inputs
    y, y2    : out std_logic);                    -- output

end g3;

architecture bool of g3 is
  signal ai, bi, ci : std_logic;
  signal yi, y2i : std_logic;
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  yi <= not (ai and bi and ci) after t;
  y2i <= (ai and bi and ci) after 2 * t;
  y <= yi;
  y2 <= y2i;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g4 is
  
  port (
    a, b, c, d : in  std_logic;               -- inputs
    y, y2    : out std_logic);                    -- output

end g4;

architecture bool of g4 is
  signal ai, bi, ci, di : std_logic;
  signal yi, y2i : std_logic;
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  di <= '1' when d = 'U' else d;
  yi <= not (ai and bi and ci and di) after t;
  y2i <= (ai and bi and ci and di) after 2 * t;
  y <= yi;
  y2 <= y2i;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g5 is
  
  port (
    a, b, c, d, e : in  std_logic;            -- inputs
    y, y2    : out std_logic);                    -- output

end g5;

architecture bool of g5 is
  signal ai, bi, ci, di, ei : std_logic;
  signal yi, y2i : std_logic;
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  di <= '1' when d = 'U' else d;
  ei <= '1' when e = 'U' else e;
  yi <= not (ai and bi and ci and di and ei) after t;
  y2i <= (ai and bi and ci and di and ei) after 2 * t;
  y <= yi;
  y2 <= y2i;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g6 is
  
  port (
    a, b, c, d, e, f : in  std_logic;         -- inputs
    y, y2    : out std_logic);                    -- output

end g6;

architecture bool of g6 is
  signal ai, bi, ci, di, ei, fi : std_logic;
  signal yi, y2i : std_logic;
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  di <= '1' when d = 'U' else d;
  ei <= '1' when e = 'U' else e;
  fi <= '1' when f = 'U' else f;
  yi <= not (ai and bi and ci and di and ei and fi) after t;
  y2i <= (ai and bi and ci and di and ei and fi) after 2 * t;
  y <= yi;
  y2 <= y2i;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity cxdriver is
  
  port (
    a : in  std_logic;                        -- source
    y : out coaxsig);                         -- destination

end cxdriver;

architecture bool of cxdriver is
  signal ai : std_logic;
  signal yi : std_logic;
begin  -- bool
  ai <= '1' when a = 'U' else a;
  yi <= ai after tc;
  y <= not (yi);                        -- coax is positive logic...
end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity cxdriver5 is
  
  port (
    a, a2, a3, a4, a5 : in  std_logic;        -- sources
    y : out coaxsig);                         -- destination

end cxdriver5;

architecture bool of cxdriver5 is
  signal ai : std_logic;
  signal yi : std_logic;
begin  -- bool
  ai <= '1' when a = 'U' else a;
  yi <= ai after tc;
  y <= not (yi);                        -- coax is positive logic...
end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity cxreceiver is
  
  port (
    a : in  coaxsig;                    -- source
    y : out std_logic);                 -- destination

end cxreceiver;

architecture bool of cxreceiver is
  signal ai : std_logic;
begin  -- bool
  ai <= '1' when a = 'U' else a;
  y <= not (ai);                        -- coax is positive logic...
end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity rsflop is
  port (
    s, r  : in  std_logic;                    -- set, reset
    s2, s3, s4, r2, r3, r4  : in  std_logic := '1';  -- extra set, reset if needed
    q, qb : out std_logic);                   -- q and q.bar

end rsflop;

architecture beh of rsflop is
  component g5
    port (
      a, b, c, d, e : in  std_logic;          -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal ri, si, qi : std_logic := '0';
begin  -- beh
  si <= s and s2 and s3 and s4;
  ri <= r and r2 and r3 and r4;
  qi <= '0' when ri = '0'
        else '1' when si = '0'
        else unaffected;
  q <= qi after t;
  qb <= not (qi) after t;

end beh;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity latch is
  
  port (
    d, clk : in  std_logic;                   -- data (set), clock
    r      : in  std_logic := '0';            -- optional reset
    q, qb  : out std_logic);                  -- q and q.bar

end latch;

architecture beh of latch is
  signal clki : std_logic;
  signal qi : std_logic;
  signal qib : std_logic;
begin  -- beh
  clki <= clk after t * 2;
  qi <= '0' when r = '1' else
      d when clki = '1' else unaffected;
  qib <= '1' when r = '1' else
       not (d) when clki = '1' else unaffected;
  q <= qi after t;
  qb <= qib after t;

end beh;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
  
entity wire is
  
  generic (
    length : integer := 0);                -- length in inches

  port (
    i  : in  std_logic;                 -- input
    o  : out std_logic);                -- output

end wire;

architecture beh of wire is
  constant feet : real := real (length) / 12.0;
  constant idelay : time := feet * 1.3 ns;
begin  -- beh

  o <= transport i after idelay;
  -- What a hack... modeling a (long) wire as an unconditional
  -- 5 ns delay doubles the speed of the GHDL simulator
  --o <= i after t;

end beh;

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

-- Common definitions for CDC 6600 model

-- signal definitions

package sigs is

  constant v0 : std_logic := '1';             -- low voltage (logic 1)
  constant v1 : std_logic := '0';             -- high voltage (logic 0)
  constant t : time := 5 ns;            -- basic stage delay
  constant tp : time := 10 ns;          -- twisted pair wire delay
  constant tc : time := 25 ns;          -- coax delay (including transistors)
  subtype coaxsigs is std_logic_vector (1 to 19);    -- CDC standard coax cable
  subtype ppword is std_logic_vector (11 downto 0);  -- PPU word (12 bits)
  type ppmem is array (0 to 4095) of ppword;  -- standard 4kx12 memory array

  function vec_to_int (
    v : std_logic_vector)                            -- input bit vector
    return integer;

  procedure dtsynchro (
    constant chnum : in    integer;     -- Channel number for this synchronizer
    incable        : inout coaxsigs;    -- Input cable
    outcable       : inout coaxsigs);   -- Output cable
  attribute foreign of dtsynchro : procedure is "VHPIDIRECT dtsynchro";

  procedure dtmain ;
  attribute foreign of dtmain : procedure is "VHPIDIRECT dtmain";

end sigs;

package body sigs is

  -- purpose: Convert a bit vector to the corresponding integer
  function vec_to_int (
    v : std_logic_vector)                            -- input bit vector
    return integer is
    variable result : integer := 0;     -- result accumulator
  begin  -- vec_to_int
    for i in v'high downto 0 loop
      result := result * 2;
      if v(i) = '1' then
        result := result + 1;
      end if;
    end loop;  -- i
    return result;
  end vec_to_int;

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
    i  : in  std_logic;                       -- input
    o  : out std_logic);                      -- output

end inv;

architecture bool of inv is
  signal ii : std_logic;
  signal oi : std_logic := '0';
begin  -- bool
  ii <= '1' when i = 'U' else i;
  oi <= not (ii) after t;
  o <= oi;

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
    x    : out std_logic);                    -- output

end g2;

architecture bool of g2 is
  signal ai, bi : std_logic;
  signal xi : std_logic := '0';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  xi <= not (ai and bi) after t;
  x <= xi;
  
end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g3 is
  
  port (
    a, b, c : in  std_logic;                  -- inputs
    x    : out std_logic);                    -- output

end g3;

architecture bool of g3 is
  signal ai, bi, ci : std_logic;
  signal xi : std_logic := '0';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  xi <= not (ai and bi and ci) after t;
  x <= xi;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g4 is
  
  port (
    a, b, c, d : in  std_logic;               -- inputs
    x    : out std_logic);                    -- output

end g4;

architecture bool of g4 is
  signal ai, bi, ci, di : std_logic;
  signal xi : std_logic := '0';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  di <= '1' when d = 'U' else d;
  xi <= not (ai and bi and ci and di) after t;
  x <= xi;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g5 is
  
  port (
    a, b, c, d, e : in  std_logic;            -- inputs
    x    : out std_logic);                    -- output

end g5;

architecture bool of g5 is
  signal ai, bi, ci, di, ei : std_logic;
  signal xi : std_logic := '0';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  di <= '1' when d = 'U' else d;
  ei <= '1' when e = 'U' else e;
  xi <= not (ai and bi and ci and di and ei) after t;
  x <= xi;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity g6 is
  
  port (
    a, b, c, d, e, f : in  std_logic;         -- inputs
    x    : out std_logic);                    -- output

end g6;

architecture bool of g6 is
  signal ai, bi, ci, di, ei, fi : std_logic;
  signal xi : std_logic := '0';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  bi <= '1' when b = 'U' else b;
  ci <= '1' when c = 'U' else c;
  di <= '1' when d = 'U' else d;
  ei <= '1' when e = 'U' else e;
  fi <= '1' when f = 'U' else f;
  xi <= not (ai and bi and ci and di and ei and fi) after t;
  x <= xi;

end bool;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity cxdriver is
  
  port (
    a : in  std_logic;                        -- source
    x : out std_logic);                       -- destination

end cxdriver;

architecture bool of cxdriver is
  signal ai : std_logic;
  signal xi : std_logic := '1';
begin  -- bool
  ai <= '1' when a = 'U' else a;
  xi <= a after tc;
  x <= xi;
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
  signal qi : std_logic := '0';
  signal qib : std_logic := '1';
begin  -- beh

  qi <= '1' when s = '0' or s2 = '0' or s3 = '0' or s4 = '0' else
        '0' when r = '0' or r2 = '0' or r3 = '0' or r4 = '0' else unaffected;
  qib<= '0' when s = '0' or s2 = '0' or s3 = '0' or s4 = '0' else
        '1' when r = '0' or r2 = '0' or r3 = '0' or r4 = '0' else unaffected;
  q <= qi after t;
  qb <= qib after t;
        
end beh;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;
entity latch is
  
  port (
    d, clk : in  std_logic;                   -- data (set), clock
    q, qb  : out std_logic);                  -- q and q.bar

end latch;

architecture beh of latch is
  signal qi : std_logic := '0';
  signal qib : std_logic := '1';
begin  -- beh

  qi <= d when clk = '1' else unaffected;
  qib <= not (d) when clk = '1' else unaffected;
  q <= qi after t;
  qb <= qib after t;

end beh;

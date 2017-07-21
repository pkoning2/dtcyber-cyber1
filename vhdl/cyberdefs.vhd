-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2017 by Paul Koning
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
  type coaxbus is array (natural range <>) of coaxsig;  -- coax cable, any width
  subtype logicsig is bit;                -- logic or tp signal
  type logicbus is array (natural range <>) of logicsig;  -- logic bus, any width

  constant t : time := 5 ns;            -- basic stage delay
  constant tc : time := 25 ns;          -- coax delay (including transistors)
  subtype coaxsigs is coaxbus (18 downto 0);    -- CDC standard coax cable
  constant idlecoax : coaxsigs := (others => '0');
  subtype tpcable is logicbus (47 downto 0);    -- CDC standard tp cable
  constant idletp : tpcable := (others => '0');
  subtype ppword is logicbus (11 downto 0);  -- PPU word (12 bits)
  subtype ppint is integer range 0 to 4095;  -- PPU word, as an integer
  subtype bankaddr is logicbus (4 downto 0);  -- Bank address (5 bits)
  subtype cpword is logicbus (59 downto 0);  -- CPU word (60 bits)
  type misc is ('x', 'y');  -- used for non-logic pins
  subtype analog is UNSIGNED (2 downto 0);    -- 6612 character drawing signal
  subtype byte is logicbus (7 downto 0);
  type bytemem is array (natural range <>) of byte;
  subtype clocks is logicbus (0 to 39);      -- system clocks
  
  procedure dtconn (
    constant chnum : in    integer;     -- Channel number for this synchronizer
    incable        : inout coaxsigs;    -- Input cable
    outcable       : inout coaxsigs);   -- Output cable
  attribute foreign of dtconn : procedure is "VHPIDIRECT dtconn";

  procedure dtmain ;
  attribute foreign of dtmain : procedure is "VHPIDIRECT dtmain";

  procedure meminit (
    constant bnum   : in    integer;      -- bank number
    constant offset : in    integer;      -- byte offset
    mbyte           : inout byte);        -- byte to initialize
  attribute foreign of meminit : procedure is "VHPIDIRECT meminit";

  procedure memwrite (
    constant bnum   : in integer;         -- bank number
    constant offset : in integer;         -- byte offset
    mbyte           : in byte);           -- byte being written
  attribute foreign of memwrite : procedure is "VHPIDIRECT memwrite";
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
  procedure dtmain is
  begin  -- dtmain
    assert false severity failure;
  end dtmain;

  -- purpose: Dummy body
  procedure meminit (
    constant bnum   : in    integer;      -- bank number
    constant offset : in    integer;      -- byte offset
    mbyte           : inout byte) is      -- byte to initialize
  begin  -- meminit
    assert false severity failure;
  end meminit;

  -- purpose: Dummy body
  procedure memwrite (
    constant bnum   : in integer;         -- bank number
    constant offset : in integer;         -- byte offset
    mbyte           : in byte) is         -- byte being written
  begin  -- memwrite
    assert false severity failure;
  end memwrite;

end sigs;

use work.sigs.all;

-- Inverter
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

use work.sigs.all;

-- inv2 is two inverters in a row, used by the 6600 for fanout
-- (or perhaps for delay)
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

-- Two input NAND gate
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

-- Three input NAND gate
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

-- Four input NAND gate
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

-- Five input NAND gate
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

-- Six input NAND gate
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

-- Coax driver.  Note that the whole coax transmission delay is
-- modeled here, at the transmit end.  So if you watch a coax signal
-- in a waveform viewer, you're seeing what corresponds to the output
-- of the coax receiver at the destination module.
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

-- Coax driver with 4 input AND inputs.
entity cxdriver4 is
  
  port (
    a, a2, a3, a4 : in  logicsig;             -- sources
    y : out coaxsig);                         -- destination

end cxdriver4;

architecture bool of cxdriver4 is
begin  -- bool
  g4: process (a, a2, a3, a4)
    variable t1 : logicsig;
  begin  -- process g4
    t1 := a and a2 and a3 and a4;
    y <= not (t1) after tc;
  end process g4;
end bool;

use work.sigs.all;

-- Coax driver with 5 input AND inputs.
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

-- Coax receiver.
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

-- R/S flip-flop.  This is modeled by behavior, not as crossed over
-- gates to make it more stable and to synthesize more efficiently.
-- R and S are active low.
entity rsflop is
  port (
    s, r  : in  logicsig;                    -- set, reset
    q, qb : out logicsig);                   -- q and q.bar

end rsflop;

architecture beh of rsflop is
  signal ri : logicsig;
begin  -- beh
  ri <= r after t;
  rsflop: process (ri, s)
    variable qi : logicsig;
  begin  -- process rsflop
    if s = '0' then
      qi := '1';
    elsif ri = '0' then
      qi := '0';
    end if;
    q <= qi after t;
    qb <= not (qi) after t;
  end process rsflop;
end beh;


use work.sigs.all;

-- R/S flip-flop with four S inputs which are logically ORed since
-- R and S are active low.
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

-- R/S flip-flop with four R and four S inputs which are logically ORed since
-- R and S are active low.
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

-- R/S flip-flop with two S inputs which are logically ORed since
-- R and S are active low.
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

-- R/S flip-flop with two R inputs which are logically ORed since
-- R and S are active low.
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

-- R/S flip-flop with two R and two S inputs which are logically ORed since
-- R and S are active low.
entity r2s2flop is
  port (
    s, r  : in  logicsig;                    -- set, reset
    s2, r2  : in  logicsig;                  -- extra set, reset
    q, qb : out logicsig);                   -- q and q.bar

end r2s2flop;

architecture beh of r2s2flop is
  component rsflop
    port (
      s, r  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal ri, si, qi : logicsig := '0';
begin  -- beh
  ri <= r and r2;
  si <= s and s2;
  u1 : rsflop port map (
    s  => si,
    r  => ri,
    q  => q,
    qb => qb);

end beh;


use work.sigs.all;

-- Latch.  This incorporates the two-inverter chain that is often
-- shown as a separate element in the diagrams, with the first
-- inverter driving the reset and the second the set input.  Some day
-- this may want to become an edge-triggered register or something
-- along those lines, but the timing is tricky in the 6600 and for now
-- we just model this entity as it was done in the original.
entity latch is
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig;
      qs : out logicsig);

end latch;
architecture gates of latch is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal ta : logicsig;
  signal tb : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => clk,
    y => ta,
    y2 => tb);


  u2 : g2 port map (
    a => d,
    b => tb,
    y => t1);

  qs <= t1;

  u3 : rsflop port map (
    r => ta,
    s => t1,
    q => q,
    qb => qb);



end gates;


use work.sigs.all;

-- Level sensitive latch with (active low) Set input.

entity latchs is
    port (
      clk : in  logicsig;
      d : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

end latchs;
architecture gates of latchs is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal ta : logicsig;
  signal tb : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => clk,
    y => ta,
    y2 => tb);


  u2 : g2 port map (
    a => d,
    b => tb,
    y => t1);


  u3 : rs2flop port map (
    r => ta,
    s => s,
    s2 => t1,
    q => q,
    qb => qb);



end gates;


use work.sigs.all;

-- Level sensitive latch with two clock inputs, which are ANDed.
entity latch2 is
  
  port (
    d, clk, clk2 : in  logicsig;                   -- data (set), clocks
    q, qb  : out logicsig);                  -- q and q.bar

end latch2;

architecture beh of latch2 is
  component latch
    port (
      d, clk : in  logicsig;                 -- data (set), clock
      q, qb, qs  : out logicsig);            -- q and q.bar
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

-- Level sensitive latch with two data inputs, which are ANDed.
entity latchd2 is
  
  port (
    d, d2, clk : in  logicsig;                   -- data (set), clock
    q, qb  : out logicsig);                  -- q and q.bar

end latchd2;

architecture beh of latchd2 is
  component latch
    port (
      d, clk : in  logicsig;                 -- data (set), clock
      q, qb, qs  : out logicsig);            -- q and q.bar
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

-- Level sensitive latch with three data inputs, which are ANDed.
entity latchd3 is
  
  port (
    d, d2, d3, clk : in  logicsig;           -- data (set), clock
    q, qb  : out logicsig);                  -- q and q.bar

end latchd3;

architecture beh of latchd3 is
  component latch
    port (
      d, clk : in  logicsig;                 -- data (set), clock
      q, qb, qs  : out logicsig);            -- q and q.bar
  end component;
  signal di : logicsig;
begin  -- beh
  di <= d and d2 and d3;
  u1 : latch port map (
    d   => di,
    clk => clk,
    q   => q,
    qb  => qb);

end beh;


use work.sigs.all;

-- Level sensitive latch with two data inputs and Set input.
entity latchd2s is
  
  port (
    d, d2, clk : in  logicsig;                   -- data (set), clock
    s : in logicsig;                    -- asynch set
    q, qb  : out logicsig);                  -- q and q.bar

end latchd2s;

architecture beh of latchd2s is
  component latchs
    port (
      d, clk : in  logicsig;                 -- data (set), clock
      s : in logicsig;                    -- asynch set
      q, qb  : out logicsig);            -- q and q.bar
  end component;
  signal di : logicsig;
begin  -- beh
  di <= d and d2;
  u1 : latchs port map (
    d   => di,
    clk => clk,
    s   => s,
    q   => q,
    qb  => qb);

end beh;


use work.sigs.all;

-- Level sensitive latch with two data inputs and a separate clock
-- for each.
entity latch22 is
  
  port (
    d, clk, d2, clk2 : in  logicsig;         -- data (set), clocks
    q, qb  : out logicsig);                  -- q and q.bar

end latch22;

architecture gates of latch22 is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component r2s2flop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal ta : logicsig;
  signal tb : logicsig;
  signal tc : logicsig;
  signal td : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => clk,
    y => ta,
    y2 => tb);


  u2 : g2 port map (
    a => d,
    b => tb,
    y => t1);

  u3 : inv2 port map (
    a => clk2,
    y => tc,
    y2 => td);


  u4 : g2 port map (
    a => d2,
    b => td,
    y => t2);


  u5 : r2s2flop port map (
    r => ta,
    r2 => tc,
    s => t1,
    s2 => t2,
    q => q,
    qb => qb);



end gates;


use work.sigs.all;

-- Level sensitive latch with and/or inputs (d and e or d2 and e2).
entity latchd4 is
  
  port (
    d, d2, e, e2, clk : in  logicsig;        -- data (set), clock
    q, qb  : out logicsig);                  -- q and q.bar

end latchd4;

architecture beh of latchd4 is
  component latch
    port (
      d, clk : in  logicsig;                 -- data (set), clock
      q, qb, qs  : out logicsig);            -- q and q.bar
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

-- Level sensitive latch with (active low) Reset
entity latchr is
  
  port (
    d, clk : in  logicsig;                   -- data (set), clock
    r      : in  logicsig;                   -- reset
    q, qb  : out logicsig);                  -- q and q.bar

end latchr;

architecture gates of latchr is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component r2sflop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal ta : logicsig;
  signal tb : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => clk,
    y => ta,
    y2 => tb);


  u2 : g2 port map (
    a => d,
    b => tb,
    y => t1);


  u3 : r2sflop port map (
    r => ta,
    r2 => r,
    s => t1,
    q => q,
    qb => qb);



end gates;


use work.sigs.all;

-- Model for a unit delay (5 ns).
entity unit_delay is

  port (
    i  : in  logicsig;                 -- input
    o  : out logicsig);                -- output

end unit_delay;

architecture beh of unit_delay is
begin  -- beh

  o <= i after 5 ns;

end beh;

-- Components used to handle a whole coax as a unit; this does not
-- appear in the 6600 wire list model but is used in non-wirelist
-- components that are modeled behaviorally, such as central memory
-- and device synchronizers.
use work.sigs.all;

entity ireg1 is
  
  port (
    clr : in bit;                       -- clear pulse
    i : in coaxsig;                     -- input
    o : out coaxsig);                   -- output

end ireg1;

architecture beh of ireg1 is
begin  -- ireg1
  -- purpose: input latch
  ilatch: process (i, clr)
  begin  -- process ilatch
    if clr = '1' then
      o <= '0';
    end if;
    if i = '1' then
        o <= '1';
    end if;
  end process ilatch;
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

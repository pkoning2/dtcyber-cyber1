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
-- PR module, rev B -- channel data register
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity prslice is
  
  port (
    d, clk : in  std_logic;                   -- data, clock
    idata  : in  coaxsig;                     -- input data coax
    r      : in  std_logic;                   -- reset
    tp     : out std_logic;                   -- test point
    odata  : out coaxsig;                     -- output data coax
    qb     : out std_logic);                  -- output (negated)

end prslice;

architecture gates of prslice is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2    : out std_logic);                  -- output
  end component;
  component cxdriver
    port (
      a : in  std_logic;                        -- source
      a2, a3, a4 : in std_logic := '1';   -- optional extra inputs
      y : out coaxsig);                         -- destination
  end component;
  component cxreceiver
    port (
      a : in  coaxsig;                    -- source
      y : out std_logic);                 -- destination
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- inputs
      s2, s3, s4, r2, r3, r4  : in  std_logic;  -- extra set, reset if needed
      q, qb : out std_logic);                 -- outputs
  end component;
  signal s  : std_logic;
  signal ii : std_logic;         -- internal copy of coax in
begin  -- gates

  u1 : g2 port map (
    a => clk,
    b => d,
    y => s);
  u2 : cxreceiver port map (
    a => idata,
    y => ii);
  u3 : rsflop port map (
    s  => s,
    s2 => ii,
    r  => r,
    q  => tp,
    qb => qb);
  u4 : cxdriver port map (
    a => s,
    y => odata);
  
end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pr is
  
  port (
    p19 : in  std_logic;               -- clock
    p14 : in  std_logic;               -- clear
    p17 : in  std_logic;               -- bit 0
    p6  : in  coaxsig := '1';                 -- coax data in bit 0
    p5  : out coaxsig;                        -- coax data out bit 0
    p8  : out std_logic;                      -- registered data 0
    p15 : in  std_logic;               -- bit 1
    p4  : in  coaxsig := '1';                 -- coax data in bit 1
    p11 : out coaxsig;                        -- coax data out bit 1
    p7  : out std_logic;                      -- registered data 1
    p16 : in  std_logic;               -- bit 2
    p23 : in  coaxsig := '1';                 -- coax data in bit 2
    p28 : out coaxsig;                        -- coax data out bit 2
    p21 : out std_logic;                      -- registered data 2
    p18 : in  std_logic;               -- bit 3
    p27 : in  coaxsig := '1';                 -- coax data in bit 3
    p24 : out coaxsig;                        -- coax data out bit 3
    p22 : out std_logic;                      -- registered data 3
    p2  : in  std_logic;
    p1  : out coaxsig;
    tp1, tp2, tp5, tp6 : out std_logic);      -- test points

end pr;

architecture gates of pr is
  component prslice
    port (
      d, clk : in  std_logic;                   -- data, clock
      idata  : in  coaxsig;                     -- input data coax
      r      : in  std_logic;                   -- reset
      tp     : out std_logic;                   -- test point
      odata  : out coaxsig;                     -- output data coax
      qb     : out std_logic);                  -- output (negated)
  end component;
  component inv
    port (
      a : in  std_logic;                      -- input
      y : out std_logic);                     -- output
  end component;
  component inv2
    port (
      a  : in  std_logic;                     -- input
      y, y2 : out std_logic);                    -- output
  end component;
  component cxdriver
    port (
      a : in  std_logic;                      -- source
      y : out coaxsig);                       -- destination
  end component;
  signal a, b : std_logic;                    -- buffered clock
begin  -- gates

  u1 : inv port map (
    a => p14,
    y => a);
  u2 : inv2 port map (
    a  => p19,
    y2 => b);
  u3 : prslice port map (
    clk   => b,
    d     => p17,
    r     => a,
    tp    => tp1,
    idata => p6,
    odata => p5,
    qb    => p8);
  u4 : prslice port map (
    clk   => b,
    d     => p15,
    r     => a,
    tp    => tp2,
    idata => p4,
    odata => p11,
    qb    => p7);
  u5 : prslice port map (
    clk   => b,
    d     => p16,
    r     => a,
    tp    => tp5,
    idata => p23,
    odata => p28,
    qb    => p21);
  u6 : prslice port map (
    clk   => b,
    d     => p18,
    r     => a,
    tp    => tp6,
    idata => p27,
    odata => p24,
    qb    => p22);
  u7 : cxdriver port map (
    a => p2,
    y => p1);
end gates;


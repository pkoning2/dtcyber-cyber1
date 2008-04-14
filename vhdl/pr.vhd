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

entity prslice is
  
  port (
    d, clk : in  std_logic;                   -- data, clock
    idata  : in  std_logic;                   -- input data coax
    r      : in  std_logic;                   -- reset
    tp     : out std_logic;                   -- test point
    odata  : out std_logic;                   -- output data coax
    qb     : out std_logic);                  -- output (negated)

end prslice;

architecture gates of prslice is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      x    : out std_logic);                  -- output
  end component;
  component cxdriver
    port (
      a : in  std_logic;                        -- source
      x : out std_logic);                       -- destination
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- inputs
      s2, s3, s4, r2, r3, r4  : in  std_logic;  -- extra set, reset if needed
      q, qb : out std_logic);                 -- outputs
  end component;
  signal s, s2, rb : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => clk,
    b => d,
    x => s);
  s2 <= s or idata;
  u3 : rsflop port map (
    s  => s2,
    r  => rb,
    q  => tp,
    qb => qb);
  u4 : cxdriver port map (
    a => s,
    x => odata);
  
end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pr is
  
  port (
    p19 : in  std_logic := '1';               -- clock
    p14 : in  std_logic := '1';               -- clear
    p17 : in  std_logic := '1';               -- bit 0
    p6  : in  std_logic := '1';               -- coax data in bit 0
    p5  : out std_logic;                      -- coax data out bit 0
    p8  : out std_logic;                      -- registered data 0
    p15 : in  std_logic := '1';               -- bit 1
    p4  : in  std_logic := '1';               -- coax data in bit 1
    p11 : out std_logic;                      -- coax data out bit 1
    p7  : out std_logic;                      -- registered data 1
    p16 : in  std_logic := '1';               -- bit 2
    p23 : in  std_logic := '1';               -- coax data in bit 2
    p28 : out std_logic;                      -- coax data out bit 2
    p21 : out std_logic;                      -- registered data 2
    p18 : in  std_logic := '1';               -- bit 3
    p27 : in  std_logic := '1';               -- coax data in bit 3
    p24 : out std_logic;                      -- coax data out bit 3
    p22 : out std_logic;                      -- registered data 3
    tp1, tp2, tp5, tp6 : out std_logic);      -- test points

end pr;

architecture gates of pr is
  component prslice
    port (
      d, clk : in  std_logic;                   -- data, clock
      idata  : in  std_logic;                   -- input data coax
      r      : in  std_logic;                   -- reset
      tp     : out std_logic;                   -- test point
      odata  : out std_logic;                   -- output data coax
      qb     : out std_logic);                  -- output (negated)
  end component;
  component inv
    port (
      i : in  std_logic;                      -- input
      o : out std_logic);                     -- output
  end component;
  signal a, b : std_logic;                    -- buffered clock
begin  -- gates

  u1 : inv port map (
    i => p14,
    o => a);
  b <= p19 after 2 * t;
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
end gates;

